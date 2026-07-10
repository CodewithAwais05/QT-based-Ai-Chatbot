#ifndef VOICERECORDER_H
#define VOICERECORDER_H

#include <QObject>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaFormat>
#include <QUrl>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>

class VoiceRecorder : public QObject
{
    Q_OBJECT

public:
    explicit VoiceRecorder(const QString& apiKey, QObject* parent = nullptr);
    ~VoiceRecorder();

    void startRecording();
    void stopRecording();
    bool getIsRecording() const;

signals:
    void transcriptionReady(const QString& text);
    void errorOccurred(const QString& error);
    void recordingStarted();
    void recordingStopped();

private slots:
    void onRecorderStateChanged(QMediaRecorder::RecorderState state);
    void onWhisperReply(QNetworkReply* reply);

private:
    void sendToWhisper(const QString& filePath);

    QMediaRecorder*        recorder;
    QMediaCaptureSession*  session;
    QAudioInput*           audioInput;
    QNetworkAccessManager* networkManager;

    QString apiKey;
    QString outputFilePath;
    bool    isRecording;
};

#endif // VOICERECORDER_H