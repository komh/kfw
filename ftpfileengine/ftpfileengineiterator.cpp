#include "ftpfileengineiterator.h"

FtpFileEngineIterator::FtpFileEngineIterator(QDir::Filters filters,
                                             const QStringList& nameFilters,
                                             const QStringList& entries)
    : QAbstractFileEngineIterator(filters, nameFilters)
    , _entries(entries)
    , _index(-1)
{
}

bool FtpFileEngineIterator::hasNext() const
{
    return _index < _entries.size() - 1;
}

QString FtpFileEngineIterator::next()
{
    if (!hasNext())
        return QString();

    ++_index;

    return currentFilePath();
}

QString FtpFileEngineIterator::currentFileName() const
{
    return _entries.at(_index);
}
