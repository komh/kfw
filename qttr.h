/****************************************************************************
**
** QtTr, a class to provide translations for Qt itself
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard.
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

#ifndef QTTR_H
#define QTTR_H

#include <QCoreApplication>

class QtTr
{
public:
    QtTr();

    static inline QString name()
    {
        return QCoreApplication::translate("QFileSystemModel", "Name");
    }

    static inline QString size()
    {
        return QCoreApplication::translate("QFileSystemModel", "Size");
    }

    static inline QString type()
    {
        return QCoreApplication::translate("QFileSystemModel", "Type");
    }

    static inline QString dateModified()
    {
        return QCoreApplication::translate("QFileSystemModel",
                                           "Date Modified");
    }
};

#endif // QTTR_H
