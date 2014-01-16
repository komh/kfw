#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QObject>
#include <QString>
#include <QFile>

class FileOperation : public QObject
{
    Q_OBJECT

public:
    FileOperation(const QString& source = QString(),
                  const QString& dest = QString(),
                  QObject* parent = 0);

    const QString& source() const { return _source; }
    const QString& dest() const { return _dest; }

    void setSource(const QString& source) { _source = source; }
    void setDest(const QString& dest) { _dest = dest; }

    enum { DefaultChunkSize = 1024 * 4 };

    bool open();
    void close();

    qint64 size() const;

    qint64 copy(qint64 chunkSize = DefaultChunkSize);

    static QString fixUrl(const QString& url);

private:
    QString _source;
    QString _dest;

    QFile _sourceFile;
    QFile _destFile;
};

#endif // FILEOPERATION_H
