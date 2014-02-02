/****************************************************************************
**
** EntryListDelegate, a delegate class to select a filename part only
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

#include "entrylistdelegate.h"

#include <QLineEdit>
#include <QEvent>
#include <QFocusEvent>

EntryListDelegate::EntryListDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

bool EntryListDelegate::eventFilter(QObject *object, QEvent *event)
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(object);

    if (lineEdit && event->type() == QEvent::FocusIn)
    {
        QFocusEvent *focus = reinterpret_cast<QFocusEvent*>(event);

        // only on initialization
        if (focus->reason() == Qt::OtherFocusReason)
        {
            QString text(lineEdit->text());

            int lastDot = text.lastIndexOf(".");
            if (lastDot == -1)
                lastDot = text.length();

            lineEdit->setSelection(0, lastDot);

            return true;
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}
