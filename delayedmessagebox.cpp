/****************************************************************************
**
** DelayedMessageBox, class to show a message with some delay
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

#include <QTimer>

#include "delayedmessagebox.h"

DelayedMessageBox::DelayedMessageBox(QWidget *parent) :
    QObject(parent)
  , _msgBox(parent)
  , _minimumDuration(500)
{
    _msgBox.setStandardButtons(QMessageBox::NoButton);
}

void DelayedMessageBox::setQuitSignal(QObject *sender, const char *signal)
{
    if (minimumDuration() == 0)
        _msgBox.open();
    else
        QTimer::singleShot(_minimumDuration, &_msgBox, SLOT(open()));

    connect(sender, signal, &_loop, SLOT(quit()), Qt::QueuedConnection);
}

void DelayedMessageBox::exec()
{
    _loop.exec();
}
