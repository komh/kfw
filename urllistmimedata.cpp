#include <QStringList>

#include "urllistmimedata.h"

UrlListMimeData::UrlListMimeData(Action action, QObject *parent) :
    QMimeData()
  , _action(action)
{
}

QStringList UrlListMimeData::formats() const
{
    if (_action == CutAction)
        return QStringList() << "text/kfw-cut-url-list";

    return QStringList() << "text/kfw-copy-url-list";
}
