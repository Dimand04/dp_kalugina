#include "barcodescanner.h"
#include <QDebug>
#include <QtConcurrent>

BarcodeScanner::BarcodeScanner(QObject *parent) : QObject(parent)
{
    m_decoder = new QZXing(this);
    m_decoder->setDecoder(QZXing::DecoderFormat_EAN_13 | QZXing::DecoderFormat_CODE_128 | QZXing::DecoderFormat_QR_CODE);
    m_decoder->setTryHarder(true);

    m_timer.start();
}

QVideoSink* BarcodeScanner::videoSink() const {
    return m_videoSink;
}

void BarcodeScanner::setVideoSink(QVideoSink *newVideoSink) {
    if (m_videoSink == newVideoSink) return;

    if (m_videoSink) {
        disconnect(m_videoSink, &QVideoSink::videoFrameChanged, this, &BarcodeScanner::processFrame);
    }

    m_videoSink = newVideoSink;

    if (m_videoSink) {
        connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &BarcodeScanner::processFrame);
    }
    emit videoSinkChanged();
}

bool BarcodeScanner::active() const {
    return m_active;
}

void BarcodeScanner::setActive(bool newActive) {
    if (m_active == newActive) return;
    m_active = newActive;
    emit activeChanged();
}

void BarcodeScanner::processFrame(const QVideoFrame &frame) {
    // 1. Простые проверки
    if (!m_active || !frame.isValid()) return;

    // 2. ЖЕСТКАЯ БЛОКИРОВКА: Если другой поток еще распознает кадр, мгновенно уходим
    bool expected = false;
    if (!m_isProcessing.compare_exchange_strong(expected, true)) {
        return;
    }

    // 3. ТРОТТЛИНГ: Проверяем кадр не чаще чем раз в 500 миллисекунд (2 раза в секунду)
    if (m_timer.elapsed() < 500) {
        m_isProcessing = false;
        return;
    }
    m_timer.restart();

    // 4. В Qt 6 toImage() сам делает map/unmap. Делаем это максимально быстро в основном потоке.
    QImage image = frame.toImage();

    if (image.isNull()) {
        m_isProcessing = false;
        return;
    }

    // 5. Уменьшаем картинку. Огромное разрешение "вешает" телефон.
    if (image.width() > 800) {
        image = image.scaledToWidth(800, Qt::FastTransformation);
    }

    // 6. ОТПРАВЛЯЕМ В ФОН: Камера больше не ждет расшифровки!
    QtConcurrent::run([this, image]() {
        QString result = m_decoder->decodeImage(image);

        if (!result.isEmpty()) {
            qDebug() << "Распознан штрихкод:" << result;
            emit barcodeDecoded(result); // Сигналы в Qt безопасно пересекают потоки
        }

        // Освобождаем блокировку, чтобы принять следующий кадр
        m_isProcessing = false;
    });
}
