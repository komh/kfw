#ifndef MOVEFILEWORKER_H
#define MOVEFILEWORKER_H

#include "abstractfileworker.h"

class MoveFileWorker : public AbstractFileWorker
{
    Q_OBJECT
public:
    explicit MoveFileWorker(const QString& source, const QString& dest,
                            QObject *parent = 0);

protected:
    virtual void performWork();
};

#endif // MOVEFILEWORKER_H
