QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    fileuploader.cpp \
    main.cpp \
    mainwindow.cpp \
    chatbot.cpp \
    message.cpp \
    filehandler.cpp \
    user.cpp \
    voicerecorder.cpp

HEADERS += \
    fileuploader.h \
    mainwindow.h \
    chatbot.h \
    message.h \
    filehandler.h \
    user.h \
    voicerecorder.h

TARGET = AICHATBOT
TEMPLATE = app
CONFIG += release
CONFIG -= debug