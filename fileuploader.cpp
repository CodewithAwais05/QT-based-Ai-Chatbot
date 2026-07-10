#include "fileuploader.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QByteArray>
#include <QDebug>
#include <QRegularExpression>

// ─── Constructor ──────────────────────────────────────────
FileUploader::FileUploader(QObject* parent) : QObject(parent) {}

// ─── detectFileType ───────────────────────────────────────
FileUploader::FileType FileUploader::detectFileType(const QString& filePath)
{
    QString ext = QFileInfo(filePath).suffix().toLower();

    if (ext == "png"  || ext == "jpg"  || ext == "jpeg" ||
        ext == "gif"  || ext == "webp" || ext == "bmp")
        return FileType::Image;

    if (ext == "txt" || ext == "md" || ext == "csv" || ext == "log")
        return FileType::TextFile;

    if (ext == "pdf")
        return FileType::PDF;

    if (ext == "docx")
        return FileType::DOCX;

    return FileType::Unknown;
}

// ─── extractTextContent ───────────────────────────────────
// Entry point: dispatches to the right extractor based on type.
QString FileUploader::extractTextContent(const QString& filePath)
{
    FileType type = detectFileType(filePath);
    switch (type) {
    case FileType::TextFile: return extractFromTxt(filePath);
    case FileType::PDF:      return extractFromPdf(filePath);
    case FileType::DOCX:     return extractFromDocx(filePath);
    default:
        return QString(); // Images are handled separately via base64
    }
}

// ─── readImageAsBase64 ────────────────────────────────────
QByteArray FileUploader::readImageAsBase64(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "FileUploader: Cannot open image:" << filePath;
        return QByteArray();
    }
    QByteArray raw = file.readAll();
    file.close();
    return raw.toBase64();
}

// ─── getMimeType ──────────────────────────────────────────
QString FileUploader::getMimeType(const QString& filePath)
{
    QString ext = QFileInfo(filePath).suffix().toLower();
    if (ext == "png")  return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif")  return "image/gif";
    if (ext == "webp") return "image/webp";
    if (ext == "bmp")  return "image/bmp";
    if (ext == "pdf")  return "application/pdf";
    if (ext == "docx") return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    return "text/plain";
}

// ─── getFileLabel ─────────────────────────────────────────
QString FileUploader::getFileLabel(const QString& filePath)
{
    QFileInfo info(filePath);
    FileType type = detectFileType(filePath);
    QString icon;
    switch (type) {
    case FileType::Image:    icon = "🖼️"; break;
    case FileType::PDF:      icon = "📄"; break;
    case FileType::DOCX:     icon = "📝"; break;
    case FileType::TextFile: icon = "📃"; break;
    default:                 icon = "📎"; break;
    }
    return icon + " " + info.fileName();
}

// ─── extractFromTxt ───────────────────────────────────────
QString FileUploader::extractFromTxt(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "FileUploader: Cannot open text file:" << filePath;
        return QString();
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();

    // Truncate very large files to avoid overwhelming the API
    const int MAX_CHARS = 12000;
    if (content.length() > MAX_CHARS) {
        content = content.left(MAX_CHARS)
        + "\n\n[... content truncated to first 12 000 characters ...]";
    }
    return content;
}

// ─── extractFromPdf ───────────────────────────────────────
// Basic PDF text extraction: scans raw bytes for readable ASCII runs.
// For a production app, link against poppler-qt or similar library.
QString FileUploader::extractFromPdf(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "FileUploader: Cannot open PDF:" << filePath;
        return QString();
    }
    QByteArray data = file.readAll();
    file.close();

    // Extract printable text runs from raw PDF bytes
    QString extracted;
    QString run;
    for (int i = 0; i < data.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(data[i]);
        if (c >= 32 && c < 127) {           // printable ASCII
            run += QChar(c);
        } else {
            if (run.length() > 4) {          // only keep meaningful runs
                extracted += run + " ";
            }
            run.clear();
        }
    }
    if (run.length() > 4) extracted += run;

    // Clean up common PDF artifacts
    extracted.replace(QRegularExpression("\\s{3,}"), " ");

    const int MAX_CHARS = 12000;
    if (extracted.length() > MAX_CHARS)
        extracted = extracted.left(MAX_CHARS)
                    + "\n\n[... PDF content truncated ...]";

    return extracted.trimmed();
}

// ─── extractFromDocx ─────────────────────────────────────
// Basic DOCX text extraction: DOCX files are ZIP archives containing
// word/document.xml. We scan for text between <w:t> tags.
// For a production app, use a proper DOCX library.
QString FileUploader::extractFromDocx(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "FileUploader: Cannot open DOCX:" << filePath;
        return QString();
    }
    QByteArray data = file.readAll();
    file.close();

    // DOCX is a ZIP — look for the XML text embedded inside.
    // We search for <w:t ...>text</w:t> patterns in the raw bytes.
    QString raw = QString::fromLatin1(data);
    QString result;
    int pos = 0;

    while (true) {
        int start = raw.indexOf("<w:t", pos);
        if (start == -1) break;
        int closeTag = raw.indexOf('>', start);
        if (closeTag == -1) break;
        int end = raw.indexOf("</w:t>", closeTag);
        if (end == -1) break;
        QString word = raw.mid(closeTag + 1, end - closeTag - 1);
        if (!word.trimmed().isEmpty())
            result += word + " ";
        pos = end + 6;
    }

    result = result.simplified();

    const int MAX_CHARS = 12000;
    if (result.length() > MAX_CHARS)
        result = result.left(MAX_CHARS)
                 + "\n\n[... DOCX content truncated ...]";

    return result.trimmed();
}