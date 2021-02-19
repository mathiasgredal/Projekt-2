#ifndef VIDEOVIEWSURFACE_H
#define VIDEOVIEWSURFACE_H
#include <iostream>

#include <QWidget>
#include <QVector2D>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QDateTime>
#include <QTimer>

// This is needed because MSVC is weird and requires you to explicitly link windows libraries
#pragma comment(lib, "wsock32")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "crypt32")
#pragma comment(lib, "advapi32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "cfgmgr32")
#pragma comment(lib, "secur32")
#pragma comment(lib, "bcrypt")

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <chrono>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#define ENABLE_LOGGING false

enum Video_Connection_Status {
    Disconnected,
    Connecting,
    Connected
};

class VideoPlayer : public QObject {
     Q_OBJECT
public:
    VideoPlayer(QGraphicsView *widget, QString url);
    ~VideoPlayer();

    void Decode_Stream();
    QVector2D Video_Size() const;

    QImage getCurrentImage();

    Video_Connection_Status status = Video_Connection_Status::Disconnected;
    std::atomic_bool pause = false;

public slots:
    void updateVideo();

private:
    QTimer *timer;
    QGraphicsView *widget;
    QGraphicsScene* scene;
    QGraphicsPixmapItem* item;
    QPixmap buffer;

    // Conversion
    AVFormatContext* stream_context;
    AVPacket packet;
    AVFormatContext* allocation_context;
    AVStream* stream = nullptr;
    AVCodec* codec;
    AVCodecContext* video_context;
    AVDictionary *d = nullptr;

    SwsContext* yuv420p_to_rgb24_ctx; // This converts YUV420P to RGB24

    std::vector<uint8_t> yuv420p_picturebuffer;
    std::vector<uint8_t> rgb24_picturebuffer;

    AVFrame* yuv420p_frame;
    AVFrame* rgb24_frame;

    int frame_count = 0;
    int video_stream_index = -1; // This is the index of the video within the h264 UDP stream

    std::thread t1;
    std::atomic_bool stop_thread = false;

    QVector2D video_size = { -1, -1 };

    // A mutex could be used instead of this janky mess
    std::atomic_bool writing_buffer = true;
    std::atomic_bool reading_buffer = false;
};

#endif // VIDEOVIEWSURFACE_H
