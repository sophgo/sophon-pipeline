//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include <QPainter>
#include <memory>
#include "video_pixmap_widget.h"
#include "ui_video_pixmap_widget.h"
#include "chrono"
#if 1
#include "libyuv.h"
#endif

#ifndef AUTO_PAINT
#define AUTO_PAINT 1 //todo: manual paint has bug.
#endif
template<typename T>
inline int intRound(const T a)
{
    return int(a+0.5f);
}

template<typename T>
inline T fastMax(const T a, const T b)
{
    return (a > b ? a : b);
}

video_pixmap_widget::video_pixmap_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::video_pixmap_widget)
{
    ui->setupUi(this);
    #if AUTO_PAINT
        m_refreshTimer = new QTimer(this);
        m_refreshTimer->setTimerType(Qt::PreciseTimer);
        connect(m_refreshTimer, SIGNAL(timeout()), this, SLOT(onRefreshTimeout()));
        m_refreshTimer->setInterval(20);
        m_refreshTimer->start();
    #endif
}

video_pixmap_widget::~video_pixmap_widget()
{
    delete ui;
}

int video_pixmap_widget::frame_width()
{
    return m_avframe == NULL ? 0:m_avframe->width;
}

int video_pixmap_widget::frame_height()
{
    return m_avframe == NULL ? 0:m_avframe->height;
}

int video_pixmap_widget::draw_frame(const AVFrame *frame, bool bUpdate)
{
    std::lock_guard<std::mutex> lck(m_syncLock);
    if (m_avframe == nullptr) {
        m_avframe = av_frame_alloc();
    }else{
        av_frame_unref(m_avframe);
    }

    av_frame_ref(m_avframe, frame);

    m_updated = true;
    if (bUpdate) {
        //QEvent *e = new QEvent(BM_UPDATE_VIDEO);
        //QCoreApplication::postEvent(this, e);
    }
    return 0;
}

int video_pixmap_widget::draw_frame(const bm::DataPtr jpeg, const bm::NetOutputDatum& datum, int h, int w)
{
    #if AUTO_PAINT
        std::lock_guard<std::mutex> lck(m_syncLock);
    #endif
    m_jpeg = jpeg;
    m_netOutputDatum = datum;
    // std::cout<<"m_jpeg = jpeg"<<std::endl;
    m_updated_lock.lock();
    m_updated = true;
    m_updated_lock.unlock();
    
    #if !AUTO_PAINT
        // std::cout<<"repaint!!!"<<std::endl;
        repaint();
        // update();
    #endif
    return 0;
}

int video_pixmap_widget::draw_info(const bm::NetOutputDatum& info, int h, int w) {
    std::lock_guard<std::mutex> lck(m_syncLock);
    m_netOutputDatum = info;
    return 0;
}


void video_pixmap_widget::paintEvent(QPaintEvent *event)
{
    // std::cout<<"paintevent!!!"<<std::endl;
    std::lock_guard<std::mutex> lck(m_syncLock);
    QPainter painter(this);    
    if (m_avframe != nullptr) {
        std::unique_ptr<uint8_t> ptr (avframe_to_rgb32(m_avframe));
        QImage origin = QImage(ptr.get(), m_avframe->width, m_avframe->height, QImage::Format_RGB32);
        if (m_netOutputDatum.type == bm::NetOutputDatum::Box) drawBox(origin);
        if (m_netOutputDatum.type == bm::NetOutputDatum::Pose) drawPose(origin);

        QImage img = origin.scaled(geometry().size(), Qt::AspectRatioMode::IgnoreAspectRatio);
        painter.drawImage(0, 0, img);
    }

#define QT_TIMER 0
#if 1//QT_TIMER
    if (m_jpeg && m_jpeg->size()> 0) { //jpeg draw
    #if QT_TIMER
        auto start_load = std::chrono::high_resolution_clock::now();
    #endif
        QImage origin;
        if(m_jpeg->image_format == FORMAT_RGB_PACKED){
            origin = QImage(m_jpeg->ptr<uint8_t>(), m_jpeg->width, m_jpeg->height, QImage::Format_RGB888);
        }else{
            origin.loadFromData(m_jpeg->ptr<uint8_t>(), m_jpeg->size());
        }
    #if QT_TIMER
        auto end_load = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_load = end_load - start_load;
        double seconds_load = 1000 * elapsed_load.count();
        std::cout << "load time: " << seconds_load << " ms" << std::endl;
    #endif

    #if QT_TIMER
        auto start_drawbox = std::chrono::high_resolution_clock::now();
    #endif
        if (m_netOutputDatum.type == bm::NetOutputDatum::Box) drawBox(origin);
        if (m_netOutputDatum.type == bm::NetOutputDatum::Pose) drawPose(origin);
    #if QT_TIMER
        auto end_drawbox = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_drawbox = end_drawbox - start_drawbox;
        double seconds_drawbox = 1000 * elapsed_drawbox.count();
        std::cout << "drawbox time: " << seconds_drawbox << " ms" << std::endl;
    #endif
    
        if(origin.width() == geometry().height() && origin.width() == geometry().width()){
        #if QT_TIMER
            auto start_drawimg = std::chrono::high_resolution_clock::now();
        #endif
            painter.drawImage(0, 0, origin); //origin
        #if QT_TIMER
            auto end_drawimg = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_drawimg = end_drawimg - start_drawimg;
            double seconds_drawimg = 1000 * elapsed_drawimg.count();
            std::cout << "drawimg time: " << seconds_drawimg << " ms" << std::endl;
        #endif
        }else{ // scale frame to geometry
        #if QT_TIMER
            auto start_scale = std::chrono::high_resolution_clock::now();
        #endif
            QImage img = origin.scaled(geometry().size(), Qt::AspectRatioMode::IgnoreAspectRatio);
        #if QT_TIMER
            auto end_scale = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_scale = end_scale - start_scale;
            double seconds_scale = 1000 * elapsed_scale.count();
            std::cout << "scale time: " << seconds_scale << " ms" << std::endl;
            auto start_drawimg = std::chrono::high_resolution_clock::now();
        #endif
            painter.drawImage(0, 0, img); //origin
        #if QT_TIMER
            auto end_drawimg = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_drawimg = end_drawimg - start_drawimg;
            double seconds_drawimg = 1000 * elapsed_drawimg.count();
            std::cout << "drawimg time: " << seconds_drawimg << " ms" << std::endl;
        #endif
        }
    }
#else
    if (m_jpeg && m_jpeg->size()> 0) { //jpeg draw
        QImage origin;
        origin.loadFromData(m_jpeg->ptr<uint8_t>(), m_jpeg->size());
    #if SAVE_QT_FRAME
        static int ii = 0;
        std::cout<<"================"<<std::endl;
        std::cout<<"save qt frame "<<ii<<std::endl;
        std::cout<<"================"<<std::endl;
        std::string img_file = "results/qt_frame_" + std::to_string(ii++) + ".jpg";
        FILE *fp = fopen(img_file.c_str(), "wb");
        fwrite(m_jpeg->ptr<uint8_t>(), m_jpeg->size(), 1, fp);
        fclose(fp);
    #endif
        if (m_netOutputDatum.type == bm::NetOutputDatum::Box) drawBox(origin);
        if (m_netOutputDatum.type == bm::NetOutputDatum::Pose) drawPose(origin);
        QImage img = origin.scaled(geometry().size(), Qt::AspectRatioMode::IgnoreAspectRatio);
        painter.drawImage(0, 0, img);
    }
#endif
}

unsigned char* video_pixmap_widget::avframe_to_rgb32(const AVFrame *src)
{
#if 0
    uint8_t /**src_data[4],*/ *dst_data[4];
    int /*src_linesize[4],*/ dst_linesize[4];
    int src_w = 320, src_h = 240, dst_w, dst_h;

    struct SwsContext *convert_ctx = NULL;
    enum AVPixelFormat src_pix_fmt = (enum AVPixelFormat)src->format;
    if (src_pix_fmt == AV_PIX_FMT_YUVJ420P) src_pix_fmt= AV_PIX_FMT_YUV420P;
    enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_RGB32;


    src_w = dst_w = src->width;
    src_h = dst_h = src->height;

    uint8_t *prgb32 = new uint8_t[src_w * src_h * 4];

    av_image_fill_arrays(dst_data, dst_linesize, prgb32, dst_pix_fmt, dst_w, dst_h, 16);
    convert_ctx = sws_getContext(src_w, src_h, src_pix_fmt, dst_w, dst_h, dst_pix_fmt,
                                 SWS_FAST_BILINEAR, NULL, NULL, NULL);

    sws_scale(convert_ctx, src->data, src->linesize, 0, dst_h,
              dst_data, dst_linesize);

    sws_freeContext(convert_ctx);

    return prgb32;
#else
    uint8_t *prgb32 = new uint8_t[src->width *src->height *4];
    // printf("NV12 = %d\n", AV_PIX_FMT_NV12);
    if (src->format == AV_PIX_FMT_YUV420P) {
        libyuv::I420ToARGB(src->data[0], src->linesize[0], src->data[1], src->linesize[1], src->data[2],
                           src->linesize[2],
                           prgb32, src->width * 4, src->width, src->height);
    }else if (AV_PIX_FMT_NV12 == src->format) {
        libyuv::NV12ToARGB(src->data[0], src->linesize[0], src->data[1], src->linesize[1],
                           prgb32, src->width * 4, src->width, src->height);
    }else if (AV_PIX_FMT_NV21 == src->format) {
        libyuv::NV21ToARGB(src->data[0], src->linesize[0], src->data[1], src->linesize[1],
                           prgb32, src->width * 4, src->width, src->height);
    }
    else{
        printf("ERROR: not support this format=%d\n", src->format);
        exit(0);
    }
    return prgb32;
#endif
}

bool video_pixmap_widget::event(QEvent *e) {
    if (e->type() == BM_UPDATE_VIDEO) {
        repaint();
        return true;
    }

    return QWidget::event(e);
}

void video_pixmap_widget::onRefreshTimeout()
{
    // std::cout<<"==============================="<<std::endl;
    // std::cout<<"Video_pixmap_widget: onRefreshTimeout"<<std::endl;
    // std::cout<<"==============================="<<std::endl;
    // auto currentTime = std::chrono::system_clock::now();
    // auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch());
    // std::time_t now_c = std::chrono::system_clock::to_time_t(currentTime);
    // std::tm now_tm = *std::localtime(&now_c);
    // char buffer[80];
    // std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_tm);
    // int milliseconds = static_cast<int>(timestamp.count() % 1000);
    // std::cout << "time now: " << buffer << "." << milliseconds << " ms" << std::endl;

    m_updated_lock.lock();
    if(m_updated){
        m_updated_lock.unlock();
        repaint();
        m_updated_lock.lock();
        m_updated = false;
    }
    m_updated_lock.unlock();
    m_roi_heatbeat++;
    if (m_roi_heatbeat > 8) {
        m_roi_heatbeat = 0;
        //m_vct_face_rect.clear();
    }
}

int video_pixmap_widget::clear_frame() {
    std::lock_guard<std::mutex> lck(m_syncLock);
    if (m_avframe) {
        av_frame_free(&m_avframe);
    }
    return 0;
}

void video_pixmap_widget::drawBox(QImage &dst)
{
    Q_ASSERT(m_netOutputDatum.type == bm::NetOutputDatum::Box);
    if (m_netOutputDatum.obj_rects.size() > 0) {
        QPainter painter1(&dst);
        QPen redPen(Qt::green);
        redPen.setWidth(3);
        painter1.setPen(redPen);
        for(int i = 0; i < m_netOutputDatum.obj_rects.size(); ++i) {

            bm::NetOutputObject pt;
            if (m_netOutputDatum.track_rects.size() > i) {
                pt = m_netOutputDatum.track_rects[i];
            }else {
                pt = m_netOutputDatum.obj_rects[i];
            }

            QRect rc(pt.x1, pt.y1, pt.x2-pt.x1, pt.y2-pt.y1);
            painter1.drawRect(rc);

            // QFont font("Arail", 40);
            // painter1.setFont(font);
            // QString text = QString("%1-%2").arg(pt.class_id).arg(pt.track_id);
            // painter1.drawText(pt.x1-1, pt.y1-4, text);

        }
    }

}

void video_pixmap_widget::drawPose(QImage &dst) {
#define POSE_BODY_25_COLORS_RENDER_GPU \
        255.f,     0.f,    85.f, \
        255.f,     0.f,     0.f, \
        255.f,    85.f,     0.f, \
        255.f,   170.f,     0.f, \
        255.f,   255.f,     0.f, \
        170.f,   255.f,     0.f, \
         85.f,   255.f,     0.f, \
          0.f,   255.f,     0.f, \
        255.f,     0.f,     0.f, \
          0.f,   255.f,    85.f, \
          0.f,   255.f,   170.f, \
          0.f,   255.f,   255.f, \
          0.f,   170.f,   255.f, \
          0.f,    85.f,   255.f, \
          0.f,     0.f,   255.f, \
        255.f,     0.f,   170.f, \
        170.f,     0.f,   255.f, \
        255.f,     0.f,   255.f, \
         85.f,     0.f,   255.f, \
          0.f,     0.f,   255.f, \
          0.f,     0.f,   255.f, \
          0.f,     0.f,   255.f, \
          0.f,   255.f,   255.f, \
          0.f,   255.f,   255.f, \
          0.f,   255.f,   255.f
#define POSE_COCO_COLORS_RENDER_GPU \
	255.f, 0.f, 85.f, \
	255.f, 0.f, 0.f, \
	255.f, 85.f, 0.f, \
	255.f, 170.f, 0.f, \
	255.f, 255.f, 0.f, \
	170.f, 255.f, 0.f, \
	85.f, 255.f, 0.f, \
	0.f, 255.f, 0.f, \
	0.f, 255.f, 85.f, \
	0.f, 255.f, 170.f, \
	0.f, 255.f, 255.f, \
	0.f, 170.f, 255.f, \
	0.f, 85.f, 255.f, \
	0.f, 0.f, 255.f, \
	255.f, 0.f, 170.f, \
	170.f, 0.f, 255.f, \
	255.f, 0.f, 255.f, \
	85.f, 0.f, 255.f
    Q_ASSERT(m_netOutputDatum.type == bm::NetOutputDatum::Pose);
    if (m_netOutputDatum.pose_keypoints.keypoints.size() == 0) return;

    std::vector<float> POSE_COCO_COLORS_RENDER;
    if (m_netOutputDatum.pose_keypoints.modeltype == bm::PoseKeyPoints::EModelType::BODY_25) {
        std::vector<float> tmp = {POSE_BODY_25_COLORS_RENDER_GPU};
        POSE_COCO_COLORS_RENDER.swap(tmp);
    } else {
        std::vector<float> tmp = {POSE_COCO_COLORS_RENDER_GPU};
        POSE_COCO_COLORS_RENDER.swap(tmp);
    }

    std::vector<unsigned int> POSE_COCO_PAIRS_RENDER;
    if (m_netOutputDatum.pose_keypoints.modeltype == bm::PoseKeyPoints::EModelType::BODY_25) {
        std::vector<unsigned int> tmp = {1, 8, 1, 2, 1, 5, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 10, 11, 8, 12, 12, 13, 13, 14, 1, 0, 0, 15, 15,
                                         17, 0, 16, 16, 18, 14, 19, 19, 20, 14, 21, 11, 22, 22, 23, 11, 24};
        POSE_COCO_PAIRS_RENDER.swap(tmp);
    } else {
        std::vector<unsigned int> tmp = {1, 2, 1, 5, 2, 3, 3, 4, 5, 6, 6, 7, 1, 8, 8, 9, 9, 10, 1, 11, 11, 12, 12, 13, 1, 0, 0, 14, 14, 16, 0, 15, 15, 17};
        POSE_COCO_PAIRS_RENDER.swap(tmp);
    }

    // Parameters
    const auto thicknessCircleRatio = 1.f / 75.f;
    const auto thicknessLineRatioWRTCircle = 0.75f;
    const auto& pairs = POSE_COCO_PAIRS_RENDER;
    const auto renderThreshold = 0.05;
    const auto scaleX = (float)dst.width()/m_netOutputDatum.pose_keypoints.width;
    const auto scaleY = (float)dst.height()/m_netOutputDatum.pose_keypoints.height;

    // Render keypoints
    renderKeyPointsCpu(dst, m_netOutputDatum.pose_keypoints.keypoints,
            m_netOutputDatum.pose_keypoints.shape, pairs, POSE_COCO_COLORS_RENDER, thicknessCircleRatio,
                       thicknessLineRatioWRTCircle, renderThreshold, scaleX, scaleY);

}

void video_pixmap_widget::renderKeyPointsCpu(QImage& img,const std::vector<float>& keypoints, std::vector<int> keyshape,
                        const std::vector<unsigned int>& pairs, const std::vector<float> colors,
                        const float thicknessCircleRatio, const float thicknessLineRatioWRTCircle,
                        const float threshold, float scaleX, float scaleY)
{
    // Get frame channels
    const auto width = img.width();
    const auto height = img.height();
    const auto area = width * height;

    // Parameters
    const auto lineType = 8;
    const auto shift = 0;
    const auto numberColors = colors.size();
    const auto thresholdRectangle = 0.1f;
    const auto numberKeypoints = keyshape[1];
    QPainter painter(&img);
    // Keypoints
    for (auto person = 0; person < keyshape[0]; person++)
    {
        {
            const auto ratioAreas = 1;
            // Size-dependent variables
            const auto thicknessRatio = fastMax(intRound(std::sqrt(area)*thicknessCircleRatio * ratioAreas), 1);
            // Negative thickness in cv::circle means that a filled circle is to be drawn.
            const auto thicknessCircle = (ratioAreas > 0.05 ? thicknessRatio : -1);
            const auto thicknessLine = 2;// intRound(thicknessRatio * thicknessLineRatioWRTCircle);
            const auto radius = thicknessRatio / 2;

            // Draw lines
            for (auto pair = 0u; pair < pairs.size(); pair += 2)
            {
                const auto index1 = (person * numberKeypoints + pairs[pair]) * keyshape[2];
                const auto index2 = (person * numberKeypoints + pairs[pair + 1]) * keyshape[2];
                if (keypoints[index1 + 2] > threshold && keypoints[index2 + 2] > threshold)
                {
                    const auto colorIndex = pairs[pair + 1] * 3; // Before: colorIndex = pair/2*3;
                    //const cv::Scalar color{ colors[(colorIndex+2) % numberColors],
                    //                        colors[(colorIndex + 1) % numberColors],
                    //                        colors[(colorIndex + 0) % numberColors] };
                    //const cv::Point keypoint1{ intRound(keypoints[index1] * scale), intRound(keypoints[index1 + 1] * scale) };
                    //const cv::Point keypoint2{ intRound(keypoints[index2] * scale), intRound(keypoints[index2 + 1] * scale) };
                    //cv::line(frame, keypoint1, keypoint2, color, thicknessLine, lineType, shift);

                    const QPoint keypoint1{ intRound(keypoints[index1] * scaleX), intRound(keypoints[index1 + 1] * scaleY) };
                    const QPoint keypoint2{ intRound(keypoints[index2] * scaleX), intRound(keypoints[index2 + 1] * scaleY) };
                    QPen pen;
                    QColor color(colors[(colorIndex+2) % numberColors],
                                        colors[(colorIndex + 1) % numberColors],
                                        colors[(colorIndex + 0) % numberColors]);
                    pen.setColor(color);
                    pen.setWidth(6);
                    painter.setPen(pen);
                    painter.drawLine(keypoint1, keypoint2);

                }
            }

            // Draw circles
            for (auto part = 0; part < numberKeypoints; part++)
            {
                const auto faceIndex = (person * numberKeypoints + part) * keyshape[2];
                if (keypoints[faceIndex + 2] > threshold)
                {
                    const auto colorIndex = part * 3;
                    //const cv::Scalar color{ colors[(colorIndex+2) % numberColors],
                    //                        colors[(colorIndex + 1) % numberColors],
                    //                        colors[(colorIndex + 0) % numberColors] };
                    //const cv::Point center{ intRound(keypoints[faceIndex] * scale), intRound(keypoints[faceIndex + 1] * scale) };
                    //cv::circle(frame, center, radius, color, thicknessCircle, lineType, shift);
                    const QPoint center{ intRound(keypoints[faceIndex] * scaleX), intRound(keypoints[faceIndex + 1] * scaleY) };
                    QPen pen;
                    QColor color(colors[(colorIndex + 2) % numberColors],
                                 colors[(colorIndex + 1) % numberColors],
                                 colors[(colorIndex + 0) % numberColors]);
                    pen.setColor(color);
                    pen.setWidth(6);
                    painter.setPen(pen);
                    painter.drawEllipse(center, radius, radius);
                }
            }
        }
    }
}