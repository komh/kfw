#ifndef REMOVEFILEWORKER_H
#define REMOVEFILEWORKER_H

#include "abstractfileworker.h"

class RemoveFileWorker : public AbstractFileWorker
{
    Q_OBJECT
public:
    explicit RemoveFileWorker(const QString& source, QObject *parent = 0);

protected:
    virtual void performWork();
};

#endif // REMOVEFILEWORKER_H
