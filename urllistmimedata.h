#ifndef URLLISTMIMEDATA_H
#define URLLISTMIMEDATA_H

#include <QMimeData>
#include <QUrl>

class UrlListMimeData : public QMimeData
{
    Q_OBJECT
public:
    enum Action { CopyAction = 1, CutAction = 2 };

    explicit UrlListMimeData(Action action = CopyAction,
                             QObject *parent = 0);

    QStringList formats() const;

    void setList(const QList<QUrl>& urlList);

    static QList<QUrl> listFrom(const QMimeData *mimeData,
                                Action action = CopyAction);

    static QString format(Action action = CopyAction);

private:
    Action _action;
};

#endif // URLLISTMIMEDATA_H
