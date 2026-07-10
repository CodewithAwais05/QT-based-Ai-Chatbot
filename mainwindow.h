#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QStackedWidget>
#include "chatbot.h"
#include "user.h"
#include "filehandler.h"
#include "voicerecorder.h"
#include "fileuploader.h"   // NEW

class MainWindow : public QMainWindow {
    Q_OBJECT
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    // ── Core objects ──────────────────────────────────────
    AIChatbot*   chatbot;
    FileHandler* fileHandler;
    User         currentUser;
    bool         isLoggedIn;

    // ── Pages ─────────────────────────────────────────────
    QStackedWidget* stackedWidget;
    QWidget*        loginPage;
    QWidget*        chatPage;
    QWidget*        historyPage;

    // ── Login page widgets ────────────────────────────────
    QLineEdit* usernameInput;
    QLineEdit* passwordInput;
    QLineEdit* emailInput;
    QLabel*    loginStatusLabel;
    QPushButton* loginBtn;
    QPushButton* registerBtn;
    bool isRegistering;

    // ── Chat page widgets ─────────────────────────────────
    QWidget*     chatContainer;
    QScrollArea* scrollArea;
    QVBoxLayout* messagesLayout;
    QTextEdit*   messageInput;
    QPushButton* sendBtn;
    QPushButton* newChatBtn;
    QPushButton* historyBtn;
    QPushButton* logoutBtn;
    QLabel*      botStatusLabel;
    QLabel*      typingLabel;
    QTimer*      typingTimer;

    // ── Voice ─────────────────────────────────────────────
    VoiceRecorder* voiceRecorder;
    QPushButton*   micBtn;
    bool           isRecording;

    // ── Theme / animation ─────────────────────────────────
    bool isDarkMode;
    int  typingDots;

    // ── Emoji picker ──────────────────────────────────────
    QPushButton* emojiBtn;
    QWidget*     emojiPicker;
    bool         emojiPickerVisible;

    // ── NEW: Upload buttons ───────────────────────────────
    QPushButton* uploadImageBtn;   // 🖼️ upload an image
    QPushButton* uploadFileBtn;    // 📎 upload a file (PDF/TXT/DOCX)

    // Pending attachment state: set when user picks a file/image,
    // cleared after the message is sent.
    QString      pendingImagePath;  // non-empty when an image is queued
    QString      pendingFilePath;   // non-empty when a doc is queued
    QLabel*      attachmentLabel;   // shows attached file name in input area

    // ── History page widgets ──────────────────────────────
    QWidget*     historyContainer;
    QVBoxLayout* historyLayout;
    QPushButton* backFromHistoryBtn;

    // ── Setup helpers ─────────────────────────────────────
    void setupLoginPage();
    void setupChatPage();
    void setupHistoryPage();
    void applyGlobalStyles();

    // ── Chat UI helpers ───────────────────────────────────
    void addMessageBubble(const QString& sender,
                          const QString& content,
                          bool isUser);
    void addImageBubble(const QString& imagePath, bool isUser); // NEW
    void scrollToBottom();
    void clearChatDisplay();
    void loadHistoryList();
    void clearPendingAttachment();   // NEW

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onSendMessage();
    void onBotResponse(const QString& response);
    void onBotError(const QString& error);
    void onTypingStarted();
    void onLoginClicked();
    void onRegisterClicked();
    void onLogoutClicked();
    void onNewChat();
    void onShowHistory();
    void onBackFromHistory();
    void updateTypingAnimation();
    void onMicButtonClicked();
    void onTranscriptionReady(const QString& text);
    void onVoiceError(const QString& error);
    void onSessionClicked(const QString& sessionId);
    void onToggleTheme();
    void onCopyMessage(const QString& text);
    void onDeleteChat();
    void onEmojiPicked(const QString& emoji);
    void onShowEmojiPicker();

    // NEW: upload slots
    void onUploadImage();
    void onUploadFile();
};

#endif // MAINWINDOW_H