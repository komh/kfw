/****************************************************************************
**
** SharedMemory, a very simple shared memory class
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

#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#ifndef QT_NO_SHAREDMEMORY
#include <QSharedMemory>
#endif

class SharedMemory
{
public:
    SharedMemory(const QString& key);
    ~SharedMemory();

    bool create(int size);
    bool attach();
    bool detach();

    bool lock(void);
    bool unlock(void);

    void* data();
    const void* data() const;

private:
#ifndef QT_NO_SHAREDMEMORY
    QSharedMemory sharedMem;
#else
    QString _key;
    void* _data;
    bool _attached;
#endif
};

#endif // SHAREDMEMORY_H
