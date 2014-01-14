#ifndef FTPFILEENGINEITERATOR_H
#define FTPFILEENGINEITERATOR_H

#include <QAbstractFileEngineIterator>

class FtpFileEngineIterator : public QAbstractFileEngineIterator
{
public:
    explicit FtpFileEngineIterator(QDir::Filters filters,
                                   const QStringList& nameFilters,
                                   const QStringList& entries);

    bool hasNext() const;
    QString next();
    QString currentFileName() const;

private:
    QStringList _entries;
    int _index;
};

#endif // FTPFILEENGINEITERATOR_H
