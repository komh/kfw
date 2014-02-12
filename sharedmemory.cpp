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

#include "sharedmemory.h"

#if defined(QT_NO_SHAREDMEMORY) && defined(Q_OS_OS2)
#define OS2EMX_PLAIN_CHAR
#define INCL_DOS
#include <os2.h>
#endif

SharedMemory::SharedMemory(const QString& key)
#ifndef QT_NO_SHAREDMEMORY
    : sharedMem(key)
#elif defined(Q_OS_OS2)
    : _key(key)
    , _data(0)
    , _attached(false)
#endif
{
}

SharedMemory::~SharedMemory()
{
    detach();
}

bool SharedMemory::create(int size)
{
#ifndef QT_NO_SHAREDMEMORY
    return sharedMem.create(size);
#elif defined(Q_OS_OS2)
    QString key("\\SHAREMEM\\");

    key.append(_key);

    _attached = DosAllocSharedMem(&_data, key.toLatin1().data(), size, fALLOC)
                    == 0;

    return _attached;
#endif
}

bool SharedMemory::attach()
{
#ifndef QT_NO_SHAREDMEMORY
    return sharedMem.attach();
#elif defined(Q_OS_OS2)
    QString key("\\SHAREMEM\\");

    key.append(_key);

    _attached = DosGetNamedSharedMem(&_data, key.toLatin1().data(), fALLOC)
                    == 0;

    return _attached;
#endif
}

bool SharedMemory::detach()
{
#ifndef QT_NO_SHAREDMEMORY
    return sharedMem.detach();
#elif defined(Q_OS_OS2)
    return _attached && DosFreeMem(_data) == 0;
#endif
}

bool SharedMemory::lock(void)
{
#ifndef QT_NO_SHAREDMEMORY
    return sharedMem.lock();
#elif defined(Q_OS_OS2)
    return _attached;
#endif
}

bool SharedMemory::unlock(void)
{
#ifndef QT_NO_SHAREDMEMORY
    return sharedMem.unlock();
#elif defined(Q_OS_OS2)
    return _attached;
#endif
}

void* SharedMemory::data()
{
#ifndef QT_NO_SHAREDMEMORY
    return sharedMem.data();
#elif defined(Q_OS_OS2)
    return _data;
#endif
}

const void* SharedMemory::data() const
{
#ifndef QT_NO_SHAREDMEMORY
    return sharedMem.data();
#elif defined(Q_OS_OS2)
    return _data;
#endif
}
