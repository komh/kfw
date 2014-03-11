/****************************************************************************
**
** DirTreeView, a class to show a directory tree
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard
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

#ifndef DIRTREEVIEW_H
#define DIRTREEVIEW_H

#include <QTreeView>

class DirTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit DirTreeView(QWidget *parent = 0);

signals:
    void dropped(const QList<QUrl>& urlList, const QString& to, bool copy);

public slots:

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    QPoint startPos;

    Qt::DropAction determineDropAction(const QPoint& pos,
                                       const Qt::KeyboardModifiers &modifiers,
                                       const QMimeData *mimeData);

    void performDrag();
};

#endif // DIRTREEVIEW_H
