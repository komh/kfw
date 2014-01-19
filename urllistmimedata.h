#ifndef URLLISTMIMEDATA_H
#define URLLISTMIMEDATA_H

#include <QMimeData>

class UrlListMimeData : public QMimeData
{
    Q_OBJECT
public:
    enum Action { CopyAction = 1, CutAction = 2 };

    explicit UrlListMimeData(Action action = CopyAction,
                             QObject *parent = 0);

    QStringList formats() const;

private:
    Action _action;
};

#endif // URLLISTMIMEDATA_H
