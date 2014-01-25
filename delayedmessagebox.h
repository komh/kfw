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

#ifndef DELAYEDMESSAGEBOX_H
#define DELAYEDMESSAGEBOX_H

#include <QMessageBox>
#include <QEventLoop>

class DelayedMessageBox : public QObject
{
    Q_OBJECT
public:
    explicit DelayedMessageBox(QWidget *parent = 0);

    QString text() const { return _msgBox.text(); }
    void setText(const QString& text) { _msgBox.setText(text); }

    QString windowTitle() const { return _msgBox.windowTitle(); }

    void setWindowTitle(const QString& title)
    {
        _msgBox.setWindowTitle(title);
    }

    int minimumDuration() const { return _minimumDuration; }
    void setMinimumDuration(int ms) { _minimumDuration = ms; }

    void setQuitSignal(QObject* sender, const char *signal);

    void exec();

signals:

public slots:

private:
    QMessageBox _msgBox;
    QEventLoop _loop;

    int _minimumDuration;
};

#endif // DELAYEDMESSAGEBOX_H
