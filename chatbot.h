#ifndef CHATBOT_H
#define CHATBOT_H

#include <QString>
#include <QVector>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "message.h"
#include "filehandler.h"
#include "fileuploader.h"   // NEW: image / file upload support

// ─── BaseChatbot ──────────────────────────────────────────
class BaseChatbot : public QObject {
    Q_OBJECT
protected:
    QString botName;
    QString sessionId;

public:
    BaseChatbot(const QString& name, QObject* parent = nullptr);
    virtual ~BaseChatbot() {}

    virtual void sendMessage(const QString& userMessage) = 0;
    virtual QString getBotName() const = 0;

signals:
    void responseReady(const QString& response);
    void errorOccurred(const QString& error);
    void typingStarted();
};

// ─── AIChatbot ────────────────────────────────────────────
class AIChatbot : public BaseChatbot {
    Q_OBJECT
private:
    QNetworkAccessManager* networkManager;
    FileHandler*   fileHandler;
    FileUploader*  fileUploader;       // NEW
    QVector<Message> conversationHistory;
    QString apiKey;
    QString currentUsername;
    bool isWaitingForResponse;

    void parseAndEmitResponse(QNetworkReply* reply);
    QJsonArray buildMessageHistory() const;

    // NEW: builds request body that may include a base64 image
    QJsonObject buildRequestBody(const QString& userMessage,
                                 const QString& imageBase64  = QString(),
                                 const QString& imageMime    = QString(),
                                 const QString& fileContext  = QString()) const;

public:
    AIChatbot(const QString& apiKey, QObject* parent = nullptr);
    ~AIChatbot();

    // Standard text message
    void sendMessage(const QString& userMessage) override;

    // NEW: send text + attached image (base64 + MIME type)
    void sendMessageWithImage(const QString& userMessage,
                              const QString& imagePath);

    // NEW: send text + extracted file text as context
    void sendMessageWithFile(const QString& userMessage,
                             const QString& filePath);

    QString getBotName() const override;

    void startNewSession(const QString& username);
    void loadSession(const QString& sessionId);
    void clearHistory();

    QVector<Message> getHistory() const;
    QString getSessionId() const;
    void setApiKey(const QString& key);

private slots:
    void onNetworkReply(QNetworkReply* reply);
};

#endif // CHATBOT_H