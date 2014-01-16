#ifndef ENTRYTREEVIEW_H
#define ENTRYTREEVIEW_H

#include <QTreeView>

#include <QtGui>

class EntryTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit EntryTreeView(QWidget *parent = 0);

signals:
    void cdUp(const QModelIndex& index);
    void paste(const QList<QUrl>&);
    void remove(const QList<QUrl>&);

public slots:

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    void copyToClipboard();
    void pasteFromClipboard();
    void deletePressed();
};

#endif // ENTRYTREEVIEW_H
