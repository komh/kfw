/****************************************************************************
**
** AddressBookDialog, an address book dialog
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

#ifndef ADDRESSBOOKDIALOG_H
#define ADDRESSBOOKDIALOG_H

#include <QDialog>

#include "serverinfo.h"

namespace Ui {
class AddressBookDialog;
}

class AddressBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddressBookDialog(QWidget *parent = 0);
    ~AddressBookDialog();

    void accept();

    QString locationUrl() const;

protected:
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_editButton_clicked();

private:

    Ui::AddressBookDialog *ui;

    QList<ServerInfo> serverInfoList;

    void setServerInfo(int row, const ServerInfo& si);
    void addServerInfo(const ServerInfo& si);
    void removeServerInfo();
    void replaceServerInfo(const ServerInfo& si);

    void loadSettings();
    void saveSettings();

    int selectedRow() const;
};

#endif // ADDRESSBOOKDIALOG_H
