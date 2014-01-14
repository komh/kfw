#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QString>

class FileOperation
{
public:
    FileOperation(const QString& source = QString(), const QString& dest = QString());

    const QString& source() const { return _source; }
    const QString& dest() const { return _dest; }

    void setSource(const QString& source) { _source = source; }
    void setDest(const QString& dest) { _dest = dest; }

    static QString fixUrl(const QString& url);

private:
    QString _source;
    QString _dest;
};

#endif // FILEOPERATION_H
