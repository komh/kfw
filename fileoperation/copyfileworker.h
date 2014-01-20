#ifndef COPYFILEWORKER_H
#define COPYFILEWORKER_H

#include "abstractfileworker.h"

class CopyFileWorker : public AbstractFileWorker
{
    Q_OBJECT
public:
    explicit CopyFileWorker(const QString& source, const QString& dest,
                            QObject *parent = 0);

protected:
    virtual void performWork();
};

#endif // COPYFILEWORKER_H
