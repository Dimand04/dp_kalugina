#ifndef BARCODESCANNER_H
#define BARCODESCANNER_H

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImage>
#include "QZXing.h"
#include <QElapsedTimer>
#include <atomic>

class BarcodeScanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

public:
    explicit BarcodeScanner(QObject *parent = nullptr);

    QVideoSink* videoSink() const;
    void setVideoSink(QVideoSink *newVideoSink);

    bool active() const;
    void setActive(bool newActive);

signals:
    void videoSinkChanged();
    void activeChanged();
    void barcodeDecoded(QString text);

private slots:
    void processFrame(const QVideoFrame &frame);

private:
    QVideoSink *m_videoSink = nullptr;
    QZXing *m_decoder = nullptr;
    bool m_active = true;
    std::atomic<bool> m_isProcessing{false};
    QElapsedTimer m_timer;
};

#endif // BARCODESCANNER_H
