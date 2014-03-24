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
#include <QApplication>

#include "delayedmessagebox.h"

DelayedMessageBox::DelayedMessageBox(QWidget *parent) :
    QObject(parent)
  , _progress(parent)
  , _minimumDuration(500)
  , _closed(true)
  , _quitSignaled(false)
{
    _progress.installEventFilter(this);

    _progress.setWindowFlags((_progress.windowFlags() |
                              Qt::CustomizeWindowHint) &
                             ~Qt::WindowCloseButtonHint);

    _progress.setCancelButton(0);
    _progress.setRange(0, 0);
}

bool DelayedMessageBox::eventFilter(QObject *target, QEvent *event)
{
    Q_UNUSED(target);

    if (event->type() == QEvent::KeyPress)
        return true;

    return false;
}

void DelayedMessageBox::setQuitSignal(QObject *sender, const char *signal)
{
    _quitSignaled = false;

    connect(sender, signal, this, SLOT(quit()));
}

void DelayedMessageBox::trigger()
{
    _closed = false;

    if (minimumDuration() == 0)
        _progress.open();
    else
        QTimer::singleShot(_minimumDuration, this, SLOT(open()));
}

void DelayedMessageBox::exec()
{
    if (!_quitSignaled)
        _loop.exec();

    _quitSignaled = false;
}

void DelayedMessageBox::close()
{
    _closed = true;

    _progress.close();
}

void DelayedMessageBox::open()
{
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());

    if (!_closed && QApplication::activeWindow() == parentWidget)
        _progress.open();
}

void DelayedMessageBox::quit()
{
    _loop.quit();

    _quitSignaled = true;
}
