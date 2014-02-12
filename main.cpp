/****************************************************************************
**
** kfw, the file manager integrating both the local and the remote
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard.
**
** $BEGIN_LICENSE$
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $END_LICENSE$
**
****************************************************************************/

#include "kfilewizard.h"

#include <QApplication>

#include "ftpfileengine/ftpfileenginehandler.h"

static QString kfwDir()
{
    QStringList pathList;

    pathList << "../kfw"        // for Qt Creator
             << "../kfw.git"    // for Qt Creator
             << ".";            // for release

    foreach(QString path, pathList)
    {
        if (QDir(path).exists())
            return path;
    }

    return QString();
}

static void initT()
{
    QString qmPath(kfwDir() + "/translations");

    QString localeName(QLocale::system().name());

    static QTranslator kfwT;    // for K File Wizard

    kfwT.load("kfw_" + localeName, qmPath);
    QApplication::installTranslator(&kfwT);

    static QTranslator qtT;     // for Qt

    qtT.load("qt_" + localeName, qmPath);
    QApplication::installTranslator(&qtT);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FtpFileEngineHandler ftpHandler;

    initT();

    KFileWizard w;
    w.show();
#ifndef QT_NO_SHAREDMEMORY
    w.lazyInitGeometry();
#endif

    return a.exec();
}
