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

#include <QProgressDialog>
#include <QEventLoop>
#include <QEvent>

class DelayedMessageBox : public QObject
{
    Q_OBJECT
public:
    explicit DelayedMessageBox(QWidget *parent = 0);

    bool eventFilter(QObject* target, QEvent *event);

    QString text() const { return _progress.labelText(); }
    void setText(const QString& text) { _progress.setLabelText(text);}

    QString windowTitle() const { return _progress.windowTitle(); }

    void setWindowTitle(const QString& title)
    {
        _progress.setWindowTitle(title);
    }

    int minimumDuration() const { return _minimumDuration; }
    void setMinimumDuration(int ms) { _minimumDuration = ms; }

    void setQuitSignal(QObject* sender, const char *signal);

    void trigger();

    void exec();

    void close();

signals:

public slots:

private:
    QProgressDialog _progress;
    QEventLoop _loop;

    int _minimumDuration;
    bool _closed;
    bool _quitSignaled;

private slots:
    void open();
    void quit();
};

#endif // DELAYEDMESSAGEBOX_H
