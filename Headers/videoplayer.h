#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include <iostream>

#include <QWidget>
#include <QVector2D>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QDateTime>
#include <QTimer>

// Link Windows libraries
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
    QString getUrl();

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
    QString videoUrl;

    // Conversion
    AVFormatContext* stream_context;
    AVPacket packet;
    AVFormatContext* allocation_context;
    AVStream* stream = nullptr;
    AVCodec* codec;
    AVCodecContext* video_context;
    AVDictionary *d = nullptr;

    SwsContext* yuv420p_to_rgb24_ctx; // Converts YUV420P to RGB24

    std::vector<uint8_t> yuv420p_picturebuffer;
    std::vector<uint8_t> rgb24_picturebuffer;

    AVFrame* yuv420p_frame;
    AVFrame* rgb24_frame;

    int frame_count = 0;
    int video_stream_index = -1; // Index of video in stream

    std::thread t1;
    std::atomic_bool stop_thread = false;

    QVector2D video_size = { -1, -1 };

    std::atomic_bool writing_buffer = true;
    std::atomic_bool reading_buffer = false;
};

#endif // VIDEOPLAYER_H
