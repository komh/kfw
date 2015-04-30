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

static void loadT(QTranslator *translator, const QString &base,
                  bool searchQtTranslationsDirFirst = false)
{
    QString localeName(QLocale::system().name());

    QStringList qmDirList;

    if (searchQtTranslationsDirFirst)
        qmDirList << QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    qmDirList << QCoreApplication::applicationDirPath();

    QString qmName(base + "_" + localeName);

    foreach(QString qmDir, qmDirList)
    {
        if (translator->load(qmName, qmDir) ||
                translator->load(qmName, qmDir + "/translations"))
        {
            QApplication::installTranslator(translator);
            break;
        }
    }
}

static void initT()
{
    // Load translations for K File Wizard
    static QTranslator kfwTrans;
    loadT(&kfwTrans, "kfw");

    // Load translations for Qt. Search in Qt translations directory first.
    static QTranslator qtTrans;
    loadT(&qtTrans, "qt", true);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    initT();

    KFileWizard w;
    w.show();
    w.lazyInitGeometry();

    return a.exec();
}
