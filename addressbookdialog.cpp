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

#include "addressbookdialog.h"
#include "ui_addressbookdialog.h"

#include "connecttodialog.h"

#include <QSettings>
#include <QKeyEvent>
#include <QMessageBox>

AddressBookDialog::AddressBookDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddressBookDialog)
{
    ui->setupUi(this);

    loadSettings();

    ui->addressTree->installEventFilter(this);
}

AddressBookDialog::~AddressBookDialog()
{
    saveSettings();

    delete ui;
}

void AddressBookDialog::accept()
{
    if (selectedRow() == -1)
    {
        QMessageBox::warning(this, tr("Address book"),
                             tr("No entry selected"));

        return;
    }

    QDialog::accept();
}

QString AddressBookDialog::locationUrl() const
{
    int row = selectedRow();

    if (row == -1)
        return QString();

    return serverInfoList.at(row).locationUrl();
}

bool AddressBookDialog::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Insert)
        {
            on_addButton_clicked();

            return true;
        }

        if (keyEvent->matches(QKeySequence::Delete))
        {
            on_removeButton_clicked();

            return true;
        }

    }

    return false;
}

void AddressBookDialog::on_addButton_clicked()
{
    ConnectToDialog dialog(this, false);

    if (dialog.exec() == QDialog::Accepted)
        addServerInfo(dialog.serverInfo());

    ui->addressTree->setFocus();
}

void AddressBookDialog::on_removeButton_clicked()
{
    removeServerInfo();

    ui->addressTree->setFocus();
}

void AddressBookDialog::on_editButton_clicked()
{
    int row = selectedRow();

    if (row == -1)
        return;

    ConnectToDialog dialog(this, false);

    dialog.setServerInfo(serverInfoList.at(row));

    if (dialog.exec() == QDialog::Accepted)
        replaceServerInfo(dialog.serverInfo());

    ui->addressTree->setFocus();
}

void AddressBookDialog::setServerInfo(int row, const ServerInfo &si)
{
    serverInfoList.replace(row, si);

    QTreeWidgetItem* header = ui->addressTree->headerItem();

    QTreeWidgetItem* item = ui->addressTree->topLevelItem(row);

    for (int col = 0; col < ui->addressTree->columnCount(); ++col)
    {
        QString text;

        if (header->text(col) == tr("Name"))
            text = si.name();
        else if (header->text(col) == tr("Host"))
            text = si.host();
        else if (header->text(col) == tr("Protocol"))
            text = ServerInfo::protocolText(si.protocol());
        else if (header->text(col) == tr("Port"))
            text = QString::number(si.port());
        else if (header->text(col) == tr("User ID"))
            text = si.userName();

        item->setText(col, text);
    }
}

void AddressBookDialog::addServerInfo(const ServerInfo &si)
{
    serverInfoList.append(si);

    int row = ui->addressTree->topLevelItemCount();

    ui->addressTree->addTopLevelItem(new QTreeWidgetItem);

    setServerInfo(row, si);
}

void AddressBookDialog::removeServerInfo()
{
    int row = selectedRow();

    if (row == -1)
        return;

    if (QMessageBox::question(this, tr("Address book"),
                              tr("Do you want remove this entry?\n\n%1")
                              .arg(serverInfoList.at(row).name()),
                              QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::No)
        return;

    serverInfoList.removeAt(row);
    ui->addressTree->takeTopLevelItem(row);
}

void AddressBookDialog::replaceServerInfo(const ServerInfo &si)
{
    int row = selectedRow();

    if (row == -1)
        return;

    setServerInfo(row, si);
}

void AddressBookDialog::loadSettings()
{
    QSettings settings;

    int size = settings.beginReadArray("serverinfo");

    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);

        ServerInfo si;

        si.setName(settings.value("name").toString());
        si.setHost(settings.value("host").toString());
        si.setProtocol(static_cast<ServerInfo::Protocol>
                            (settings.value("protocol").toInt()));
        si.setPort(settings.value("port").toInt());
        si.setTransferMode(static_cast<ServerInfo::TransferMode>
                            (settings.value("transfermode").toInt()));
        si.setEncoding(static_cast<ServerInfo::Encoding>
                            (settings.value("encoding").toInt()));
        si.setAnonymous(settings.value("anonymous").toBool());
        si.setUserName(settings.value("userid").toString());
        si.setPassword(settings.value("password").toString());
        si.setDirectory(settings.value("directory").toString());

        addServerInfo(si);
    }

    settings.endArray();
}

void AddressBookDialog::saveSettings()
{
    QSettings settings;

    settings.beginWriteArray("serverinfo");

    for (int i = 0; i < serverInfoList.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("name", serverInfoList.at(i).name());
        settings.setValue("host", serverInfoList.at(i).host());
        settings.setValue("protocol", serverInfoList.at(i).protocol());
        settings.setValue("port", serverInfoList.at(i).port());
        settings.setValue("transfermode", serverInfoList.at(i).transferMode());
        settings.setValue("encoding", serverInfoList.at(i).encoding());
        settings.setValue("anonymous", serverInfoList.at(i).isAnonymous());
        settings.setValue("userid", serverInfoList.at(i).userName());
        settings.setValue("password", serverInfoList.at(i).password());
        settings.setValue("directory", serverInfoList.at(i).directory());
    }

    settings.endArray();
}

int AddressBookDialog::selectedRow() const
{
    QList<QTreeWidgetItem*>
            range(ui->addressTree->selectedItems());

    if (range.isEmpty())
        return -1;

    return ui->addressTree->indexOfTopLevelItem(range.first());
}
