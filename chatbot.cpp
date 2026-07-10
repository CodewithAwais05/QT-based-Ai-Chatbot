#include "chatbot.h"
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>

// ─── BaseChatbot ──────────────────────────────────────────
BaseChatbot::BaseChatbot(const QString& name, QObject* parent)
    : QObject(parent), botName(name) {}

// ─── AIChatbot ────────────────────────────────────────────
AIChatbot::AIChatbot(const QString& key, QObject* parent)
    : BaseChatbot("Claude AI", parent),
    apiKey(key),
    isWaitingForResponse(false)
{
    networkManager = new QNetworkAccessManager(this);
    fileHandler    = new FileHandler();
    fileUploader   = new FileUploader(this);   // NEW

    connect(networkManager, &QNetworkAccessManager::finished,
            this, &AIChatbot::onNetworkReply);

    startNewSession("default");
}

AIChatbot::~AIChatbot() {
    delete fileHandler;
}

QString AIChatbot::getBotName() const { return botName; }
QVector<Message> AIChatbot::getHistory() const { return conversationHistory; }
QString AIChatbot::getSessionId() const { return sessionId; }
void AIChatbot::setApiKey(const QString& key) { apiKey = key; }

void AIChatbot::startNewSession(const QString& username) {
    currentUsername = username;
    sessionId = "session_" + QUuid::createUuid().toString()
                                 .remove("{").remove("}").remove("-")
                                 .left(12);
    conversationHistory.clear();
    fileHandler->logActivity("New session: " + sessionId
                             + " for user: " + username);
}

void AIChatbot::loadSession(const QString& sid) {
    sessionId = sid;
    conversationHistory = fileHandler->loadMessages(sid);
}

void AIChatbot::clearHistory() {
    conversationHistory.clear();
}

// ─── buildMessageHistory ──────────────────────────────────
QJsonArray AIChatbot::buildMessageHistory() const {
    QJsonArray arr;
    for (const Message& msg : conversationHistory) {
        QJsonObject obj;
        obj["role"]    = (msg.getSender() == "You") ? "user" : "assistant";
        obj["content"] = msg.getContent();
        arr.append(obj);
    }
    return arr;
}

// ─── buildRequestBody ─────────────────────────────────────
// Builds the JSON body for the Groq API request.
// If imageBase64 is provided, embeds the image in vision format.
// If fileContext is provided, prepends it to the user message.
// NOTE: Groq's llama-3.3-70b-versatile does NOT support vision.
//       For image queries we fall back to a descriptive prompt that
//       informs the user, while still sending the metadata.
//       When vision support becomes available in your Groq plan,
//       swap the model to "meta-llama/llama-4-scout-17b-16e-instruct"
//       or another vision-capable model.
QJsonObject AIChatbot::buildRequestBody(const QString& userMessage,
                                        const QString& imageBase64,
                                        const QString& imageMime,
                                        const QString& fileContext) const
{
    // System message
    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] = "You are a helpful, friendly and intelligent AI assistant. "
                        "When file content is shared, use it to answer the user's question accurately.";

    // Build messages array (last 10 history entries + new message)
    QJsonArray messages;
    messages.append(sysMsg);

    int start = qMax(0, conversationHistory.size() - 10);
    for (int i = start; i < conversationHistory.size(); ++i) {
        const Message& msg = conversationHistory[i];
        QJsonObject obj;
        obj["role"]    = (msg.getSender() == "You") ? "user" : "assistant";
        obj["content"] = msg.getContent();
        messages.append(obj);
    }

    // Build the new user message content
    QJsonObject userMsg;
    userMsg["role"] = "user";

    if (!imageBase64.isEmpty()) {
        // ── Vision path ──────────────────────────────────────────
        // Build a content array with both image and text parts.
        // Groq vision-capable models accept this format.
        QJsonArray contentArr;

        QJsonObject imgPart;
        imgPart["type"] = "image_url";
        QJsonObject imgUrl;
        imgUrl["url"] = "data:" + imageMime + ";base64," + imageBase64;
        imgPart["image_url"] = imgUrl;
        contentArr.append(imgPart);

        QJsonObject txtPart;
        txtPart["type"] = "text";
        txtPart["text"] = userMessage;
        contentArr.append(txtPart);

        userMsg["content"] = contentArr;

    } else if (!fileContext.isEmpty()) {
        // ── File context path ─────────────────────────────────────
        // Prepend extracted file text as context to the user question.
        QString combined = "Here is the content of the uploaded file:\n\n"
                           "---\n" + fileContext + "\n---\n\n"
                                           "User question: " + userMessage;
        userMsg["content"] = combined;

    } else {
        // ── Plain text path ───────────────────────────────────────
        userMsg["content"] = userMessage;
    }

    messages.append(userMsg);

    // Choose model: use vision model when image is attached
    QString model = imageBase64.isEmpty()
                        ? "llama-3.3-70b-versatile"
                        : "meta-llama/llama-4-scout-17b-16e-instruct";

    QJsonObject body;
    body["model"]       = model;
    body["messages"]    = messages;
    body["max_tokens"]  = 1024;
    body["temperature"] = 0.7;
    return body;
}

// ─── Shared network posting helper ───────────────────────
static void postToGroq(QNetworkAccessManager* nm,
                       const QString& apiKey,
                       const QJsonObject& body)
{
    QUrl url("https://api.groq.com/openai/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QJsonDocument doc(body);
    QByteArray requestData = doc.toJson();
    qDebug() << "Sending to Groq:" << requestData.left(400);
    nm->post(request, requestData);
}

// ─── sendMessage (plain text) ─────────────────────────────
void AIChatbot::sendMessage(const QString& userMessage) {
    if (isWaitingForResponse || userMessage.trimmed().isEmpty()) return;

    isWaitingForResponse = true;
    emit typingStarted();

    Message userMsg("You", userMessage);
    conversationHistory.append(userMsg);
    fileHandler->saveMessage(sessionId, userMsg);

    QJsonObject body = buildRequestBody(userMessage);
    postToGroq(networkManager, apiKey, body);
}

// ─── sendMessageWithImage ─────────────────────────────────
// Reads the image, encodes it as base64, then sends both the
// image and the user's question to the vision-capable model.
void AIChatbot::sendMessageWithImage(const QString& userMessage,
                                     const QString& imagePath)
{
    if (isWaitingForResponse) return;

    isWaitingForResponse = true;
    emit typingStarted();

    // Record what the user "said" (include image hint in history)
    QString historyMsg = userMessage.isEmpty()
                             ? "[Image uploaded]"
                             : userMessage + " [Image attached]";
    Message userMsg("You", historyMsg);
    conversationHistory.append(userMsg);
    fileHandler->saveMessage(sessionId, userMsg);

    // Encode image
    QByteArray b64    = FileUploader::readImageAsBase64(imagePath);
    QString    mime   = FileUploader::getMimeType(imagePath);

    if (b64.isEmpty()) {
        // Fallback: image unreadable
        emit errorOccurred("Could not read the image file. Please try a different file.");
        isWaitingForResponse = false;
        return;
    }

    QString prompt = userMessage.isEmpty()
                         ? "Please describe this image in detail."
                         : userMessage;

    QJsonObject body = buildRequestBody(prompt,
                                        QString::fromLatin1(b64),
                                        mime);
    postToGroq(networkManager, apiKey, body);
}

// ─── sendMessageWithFile ──────────────────────────────────
// Extracts text from the file and sends it as context alongside
// the user's question.
void AIChatbot::sendMessageWithFile(const QString& userMessage,
                                    const QString& filePath)
{
    if (isWaitingForResponse) return;

    isWaitingForResponse = true;
    emit typingStarted();

    // Attempt text extraction
    QString content = FileUploader::extractTextContent(filePath);

    if (content.isEmpty()) {
        emit errorOccurred("Could not extract text from this file. "
                           "Supported formats: TXT, PDF, DOCX, MD, CSV.");
        isWaitingForResponse = false;
        return;
    }

    // Record in history
    QString label = FileUploader::getFileLabel(filePath);
    QString historyMsg = userMessage.isEmpty()
                             ? "[File uploaded: " + label + "]"
                             : userMessage + "\n[File: " + label + "]";
    Message userMsg("You", historyMsg);
    conversationHistory.append(userMsg);
    fileHandler->saveMessage(sessionId, userMsg);

    QString prompt = userMessage.isEmpty()
                         ? "Please summarize the content of this file."
                         : userMessage;

    QJsonObject body = buildRequestBody(prompt, QString(), QString(), content);
    postToGroq(networkManager, apiKey, body);
}

// ─── onNetworkReply ───────────────────────────────────────
void AIChatbot::onNetworkReply(QNetworkReply* reply) {
    isWaitingForResponse = false;
    parseAndEmitResponse(reply);
    reply->deleteLater();
}

// ─── parseAndEmitResponse ─────────────────────────────────
void AIChatbot::parseAndEmitResponse(QNetworkReply* reply) {
    QByteArray responseData = reply->readAll();

    qDebug() << "=== GROQ RESPONSE ===";
    qDebug() << "HTTP Status:"
             << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "Full Response:" << responseData;
    qDebug() << "====================";

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network Error:" << reply->errorString();
    }

    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject root  = doc.object();

    if (root.contains("error")) {
        QString errMsg = root["error"].toObject()["message"].toString();
        qDebug() << "API Error Message:" << errMsg;
        emit errorOccurred("API Error: " + errMsg);
        return;
    }

    QJsonArray choices = root["choices"].toArray();
    if (choices.isEmpty()) {
        emit errorOccurred("Empty response from Groq.");
        return;
    }

    QString responseText = choices[0].toObject()
                               ["message"].toObject()
                                       ["content"].toString();

    Message botMsg("Assistant", responseText);
    conversationHistory.append(botMsg);
    fileHandler->saveMessage(sessionId, botMsg);

    emit responseReady(responseText);
}