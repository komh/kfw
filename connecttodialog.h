/****************************************************************************
**
** ConnectToDialog, connectTo dialog class
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

#ifndef CONNECTTODIALOG_H
#define CONNECTTODIALOG_H

#include <QDialog>

namespace Ui {
class ConnectToDialog;
}

class ConnectToDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectToDialog(QWidget *parent = 0, bool connectMode = true);
    ~ConnectToDialog();

    QString name() const;
    QString protocol() const;
    QString host() const;
    int port() const;
    QString transferMode() const;
    QString encoding() const;
    bool isAnonymous() const;
    QString userName() const;
    QString password() const;
    QString directory() const;

    QString locationUrl() const;

private:
    Ui::ConnectToDialog *ui;

private slots:
    void anonymousStateChanged(int state);
};

#endif // CONNECTTODIALOG_H
