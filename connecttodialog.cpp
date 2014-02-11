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

    connect(ui->anonymousCheck, SIGNAL(stateChanged(int)),
            this, SLOT(anonymousStateChanged(int)));

    ui->anonymousCheck->setChecked(true);
}

ConnectToDialog::~ConnectToDialog()
{
    delete ui;
}

QString ConnectToDialog::name() const
{
    return ui->nameLine->text();
}

QString ConnectToDialog::protocol() const
{
    return ui->protocolCombo->currentText();
}

QString ConnectToDialog::host() const
{
    return ui->hostLine->text();
}

int ConnectToDialog::port() const
{
    return ui->portSpin->value();
}

QString ConnectToDialog::transferMode() const
{
    return ui->transferModeCombo->currentText();
}

QString ConnectToDialog::encoding() const
{
    return ui->encodingCombo->currentText();
}

bool ConnectToDialog::isAnonymous() const
{
    return ui->anonymousCheck->isChecked();
}

QString ConnectToDialog::userName() const
{
    return ui->userNameLine->text();
}

QString ConnectToDialog::password() const
{
    return ui->passwordLine->text();
}

QString ConnectToDialog::directory() const
{
    return ui->directoryLine->text();
}

QString ConnectToDialog::locationUrl() const
{
    QString url;

    url.append(protocol().toLower());
    url.append("://");

    if (!isAnonymous() && userName() != "anonymous")
    {
        url.append(userName());

        if (!password().isEmpty())
        {
            url.append(":");
            url.append(password());
        }

        url.append("@");
    }

    url.append(host());

    if (port() != 21)
    {
        url.append(":");
        url.append(QString::number(port()));
    }

    url.append(directory());

    return url;
}

void ConnectToDialog::anonymousStateChanged(int state)
{
    bool disabled = state == Qt::Checked;

    ui->userNameLabel->setDisabled(disabled);
    ui->userNameLine->setDisabled(disabled);
    ui->passwordLabel->setDisabled(disabled);
    ui->passwordLine->setDisabled(disabled);
}
