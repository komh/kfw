#ifndef RENAMEFILEWORKER_H
#define RENAMEFILEWORKER_H

#include "abstractfileworker.h"

class RenameFileWorker : public AbstractFileWorker
{
    Q_OBJECT
public:
    explicit RenameFileWorker(const QString& source, const QString& dest,
                              QObject *parent = 0);

protected:
    virtual void performWork();
};

#endif // RENAMEFILEWORKER_H
