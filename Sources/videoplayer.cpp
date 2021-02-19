#include "../Headers/videoviewsurface.h"

void Log(void* ptr, int level, const char* fmt, va_list vargs)
{
    static char message[8192];
    const char* module = NULL;

    if (ptr) {
        AVClass* avc = *(AVClass**)ptr;
        module = avc->item_name(ptr);
    }
    vsnprintf(message, sizeof(message), fmt, vargs);

    std::cout << "LOG: " << message << std::endl;
}

VideoPlayer::VideoPlayer(QGraphicsView *widget, QString url) : widget(widget)
{
    widget->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    widget->setCacheMode(QGraphicsView::CacheBackground);
    widget->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    widget->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    scene = new QGraphicsScene(this);
    widget->setScene(scene);


    //Initialize video decoding
    stream_context = avformat_alloc_context();

    // Set options for rtsp
    if(url.contains("rtsp")) {
        stream_context->max_analyze_duration = 20;
        stream_context->probesize = 20;
        // We assume all rtsp streams are tcp based.
        av_dict_set(&d, "rtsp_transport", "tcp", 0);
     }

    av_register_all();
    avformat_network_init();

#if ENABLE_LOGGING
    av_log_set_level(AV_LOG_WARNING);
    av_log_set_callback(&Log);
#endif

    if (avformat_open_input(&stream_context, url.toStdString().c_str(), NULL, &d))
        throw std::runtime_error("ERROR: Unable to connect to url: " + url.toStdString());

    if (avformat_find_stream_info(stream_context, NULL) < 0)
        throw std::runtime_error("ERROR: Unable to find stream info");

    for (uint32_t i = 0; i < stream_context->nb_streams; i++) {
        if (stream_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_stream_index = i;
    }

    if (video_stream_index == -1)
        throw std::runtime_error("ERROR: Unable to find video in requested stream");

    av_init_packet(&packet);

    allocation_context = avformat_alloc_context();

    // Now we can start reading the video packets
    av_read_play(stream_context);

    // We assume h264 frames
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec)
        throw std::runtime_error("ERROR: The linked ffmpeg library seems not to have an h264 decoder");

    video_context = avcodec_alloc_context3(codec);
    avcodec_copy_context(video_context, stream_context->streams[video_stream_index]->codec);

    if (avcodec_open2(video_context, codec, nullptr) < 0)
        throw std::runtime_error("ERROR: Unable to initialize the video context with the given AVCodec");

    std::cout << "Pixel format: " << video_context->pix_fmt << std::endl;
    // Initializing the converter that will convert the decoded YUV420P frames to RGB24 frames that we can render with SFML
    yuv420p_to_rgb24_ctx = sws_getContext(video_context->width, video_context->height, video_context->pix_fmt, video_context->width, video_context->height,
        AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

    int yuv420p_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, video_context->width, video_context->height, 1);
    yuv420p_picturebuffer.resize(yuv420p_size);

    int rgb24_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, video_context->width, video_context->height, 1);
    rgb24_picturebuffer.resize(rgb24_size);

    yuv420p_frame = av_frame_alloc();
    av_image_fill_arrays(yuv420p_frame->data, yuv420p_frame->linesize, yuv420p_picturebuffer.data(), AV_PIX_FMT_YUV420P, video_context->width, video_context->height, 1);

    rgb24_frame = av_frame_alloc();
    av_image_fill_arrays(rgb24_frame->data, rgb24_frame->linesize, rgb24_picturebuffer.data(), AV_PIX_FMT_RGB24, video_context->width, video_context->height, 1);

    video_size = { (float)video_context->width, (float)video_context->height };

    // Now most of the preliminary work is done, so now we can fire up a new thread for decoding
    t1 = std::thread([this]() { Decode_Stream(); });

    // We use a timer to update the widget at 30 fps
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateVideo()));
    timer->start(33); // Time in milliseconds
}

VideoPlayer::~VideoPlayer()
{
    // Close thread
    stop_thread = true;
    t1.join();

    // Clear heap
    av_dict_free(&d);
    av_free(yuv420p_frame);
    av_free(rgb24_frame);
    av_read_pause(stream_context);
    avio_close(allocation_context->pb);
    avformat_free_context(allocation_context);
    avformat_close_input(&stream_context);
    avformat_network_deinit();

    std::cout << "Video destroyed" << std::endl;
}

QImage VideoPlayer::getCurrentImage()
{
    // Wait until the decoder thread has stopped writing to the buffer
    while (writing_buffer) {
    }

    reading_buffer = true;
    const QImage tempImage = QImage(rgb24_picturebuffer.data(), video_context->width, video_context->height, QImage::Format_RGB888);
    reading_buffer = false;

    return tempImage;
}

void VideoPlayer::updateVideo()
{
    if(pause)
        return;

    buffer = QPixmap::fromImage(getCurrentImage());

    if(!buffer.isNull()) {
        scene->clear();
        scene->addPixmap(buffer);
        scene->setSceneRect(0,0, buffer.width(), buffer.height());
        widget->fitInView(scene->sceneRect(),Qt::KeepAspectRatio);
        widget->update();
    } else {
        std::cout << "ERROR: Invalid pixmap" << std::endl;
    }

}

void VideoPlayer::Decode_Stream()
{
    uint64_t time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

     while (av_read_frame(stream_context, &packet) >= 0 && stop_thread != true) {
         // Check whether this packet belongs to the videostream
         if (packet.stream_index == video_stream_index && pause != true) {

             if (stream == nullptr) { // This will only be run once
                 stream = avformat_new_stream(allocation_context, stream_context->streams[video_stream_index]->codec->codec);
                 avcodec_copy_context(stream->codec, stream_context->streams[video_stream_index]->codec);
                 stream->sample_aspect_ratio = stream_context->streams[video_stream_index]->codec->sample_aspect_ratio;
             }

             int check = 0;
             packet.stream_index = stream->id;

             avcodec_decode_video2(video_context, yuv420p_frame, &check, &packet);

             if (check) {
                 while (reading_buffer) {
                 }

                 writing_buffer = true;
                 // We have succesfully decoded a frame
                 sws_scale(yuv420p_to_rgb24_ctx, yuv420p_frame->data, yuv420p_frame->linesize, 0, video_context->height, rgb24_frame->data, rgb24_frame->linesize);
                 writing_buffer = false;

                 // Update sfml video image thing
                 std::cout << "Frame: " << frame_count << ", FPS: " << frame_count / (float)(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() - time) << std::endl;
                 frame_count++;
             }
         }

         av_packet_unref(&packet);
         av_init_packet(&packet);
     }
}

QVector2D VideoPlayer::Video_Size() const
{
    return video_size;
}
