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

#include "connecttodialog.h"
#include "ui_connecttodialog.h"

#include <QPushButton>
#include <QMessageBox>

ConnectToDialog::ConnectToDialog(QWidget *parent, bool connectMode) :
    QDialog(parent),
    ui(new Ui::ConnectToDialog)
{
    ui->setupUi(this);

    if (connectMode)
    {
        ui->nameLabel->setDisabled(true);
        ui->nameLine->setDisabled(true);

        ui->hostLine->setFocus(Qt::OtherFocusReason);

        ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Connect"));
    }

    ui->protocolCombo->addItems(ServerInfo::protocolList());
    ui->transferModeCombo->addItems(ServerInfo::transferModeList());
    ui->encodingCombo->addItems(ServerInfo::encodingList());

    connect(ui->anonymousCheck, SIGNAL(stateChanged(int)),
            this, SLOT(anonymousStateChanged(int)));

    ui->anonymousCheck->setChecked(true);
}

ConnectToDialog::~ConnectToDialog()
{
    delete ui;
}

void ConnectToDialog::accept()
{
    bool connectMode = !ui->nameLine->isEnabled();

    if (!connectMode && ui->nameLine->text().isEmpty())
    {
        QMessageBox::warning(this, windowTitle(),
                             tr("%1 field is empty").arg(tr("Name:")));

        ui->nameLine->setFocus();

        return;
    }

    if (ui->hostLine->text().isEmpty())
    {
        QMessageBox::warning(this, windowTitle(),
                             tr("%1 field is empty").arg(tr("Host:")));

        ui->hostLine->setFocus();

        return;
    }

    _serverInfo.setName(ui->nameLine->text());
    _serverInfo.setHost(ui->hostLine->text());
    _serverInfo.setProtocol(ui->protocolCombo->currentText());
    _serverInfo.setPort(ui->portSpin->value());
    _serverInfo.setTransferMode(ui->transferModeCombo->currentText());
    _serverInfo.setEncoding(ui->encodingCombo->currentText());
    _serverInfo.setAnonymous(ui->anonymousCheck->isChecked());
    _serverInfo.setUserName(ui->userNameLine->text());
    _serverInfo.setPassword((ui->passwordLine->text()));
    _serverInfo.setDirectory(ui->directoryLine->text());

    QDialog::accept();
}

QString ConnectToDialog::locationUrl() const
{
    return serverInfo().locationUrl();
}

const ServerInfo &ConnectToDialog::serverInfo() const
{
    return _serverInfo;
}

void ConnectToDialog::setServerInfo(const ServerInfo& si)
{
    _serverInfo = si;

    ui->nameLine->setText(si.name());
    ui->hostLine->setText(si.host());
    ui->protocolCombo->setCurrentIndex(
                ui->protocolCombo->findText(
                    ServerInfo::protocolText(si.protocol())));
    ui->portSpin->setValue(si.port());
    ui->transferModeCombo->setCurrentIndex(
                ui->transferModeCombo->findText(
                    ServerInfo::transferModeText(si.transferMode())));
    ui->encodingCombo->setCurrentIndex(
                ui->encodingCombo->findText(
                    ServerInfo::encodingText(si.encoding())));
    ui->anonymousCheck->setChecked(si.isAnonymous());
    ui->userNameLine->setText(si.userName());
    ui->passwordLine->setText(si.password());
    ui->directoryLine->setText(si.directory());
}

void ConnectToDialog::anonymousStateChanged(int state)
{
    bool disabled = state == Qt::Checked;

    ui->userNameLabel->setDisabled(disabled);
    ui->userNameLine->setDisabled(disabled);
    ui->passwordLabel->setDisabled(disabled);
    ui->passwordLine->setDisabled(disabled);
}
