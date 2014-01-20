#include <QThread>

#include "abstractfileworker.h"

AbstractFileWorker::AbstractFileWorker(const QString &source,
                                       const QString &dest, QObject *parent) :
    QObject(parent)
  , _source(source)
  , _dest(dest)
  , _canceled(false)
  , _result(false)
{
}

void AbstractFileWorker::perform()
{
    performWork();

    QThread::currentThread()->quit();
}
