#include <QtGui>

#include "entrytreeview.h"
#include "entrylistmodel.h"
#include "filesystemsortfilterproxymodel.h"

EntryTreeView::EntryTreeView(QWidget *parent) :
    QTreeView(parent)
{
}

void EntryTreeView::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Back))
    {
        emit cdUp(rootIndex());

        return;
    }

    QTreeView::keyPressEvent(event);
}
