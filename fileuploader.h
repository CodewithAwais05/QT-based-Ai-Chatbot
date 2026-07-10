#ifndef FILEUPLOADER_H
#define FILEUPLOADER_H

#include <QObject>
#include <QString>
#include <QByteArray>

// ─── FileUploader ─────────────────────────────────────────
// Handles reading and extracting content from uploaded files.
// Supports: images (PNG, JPG, GIF, WEBP), text (TXT, MD, CSV),
//           PDF (basic), DOCX (basic text extraction).
class FileUploader : public QObject
{
    Q_OBJECT

public:
    enum class FileType {
        Image,      // PNG, JPG, JPEG, GIF, WEBP, BMP
        TextFile,   // TXT, MD, CSV, LOG
        PDF,        // PDF (basic text extraction)
        DOCX,       // DOCX (basic XML text extraction)
        Unknown
    };

    explicit FileUploader(QObject* parent = nullptr);

    // Detect file type from extension
    static FileType detectFileType(const QString& filePath);

    // Extract plain text content from a file (for TXT/PDF/DOCX)
    static QString extractTextContent(const QString& filePath);

    // Read image as base64-encoded string (for API submission)
    static QByteArray readImageAsBase64(const QString& filePath);

    // Get the MIME type string for a given file path
    static QString getMimeType(const QString& filePath);

    // Returns a short description of the file for display in chat
    static QString getFileLabel(const QString& filePath);

private:
    static QString extractFromTxt(const QString& filePath);
    static QString extractFromPdf(const QString& filePath);
    static QString extractFromDocx(const QString& filePath);
};

#endif // FILEUPLOADER_H