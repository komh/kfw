/****************************************************************************
**
** EntryTreeView, class in order to show directory entires
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard.
**
** $BEGIN_LICENSE$
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** $END_LICENSE$
**
****************************************************************************/

#ifndef ENTRYTREEVIEW_H
#define ENTRYTREEVIEW_H

#include <QTreeView>

#include <QtGui>

class EntryTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit EntryTreeView(QWidget *parent = 0);

    QList<QUrl> selectedUrlList();

signals:
    void cdUp(const QModelIndex& index);
    void paste(const QList<QUrl>&, bool copy = true);
    void remove(const QList<QUrl>&);
    void refresh();
    void dropped(const QList<QUrl>& urlList, const QString& to, bool copy);

public slots:

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);

private:
    QPoint startPos;

    void copyToClipboard(bool copy = true);
    void pasteFromClipboard();
    void deletePressed();

    void perfromDrag();
    Qt::DropAction determineDropAction(const QPoint &pos,
                                       const Qt::KeyboardModifiers &modifier,
                                       const QMimeData *mimeData);
};

#endif // ENTRYTREEVIEW_H
