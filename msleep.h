#ifndef MSLEEP_H
#define MSLEEP_H

#include <QThread>

class MSleep : public QThread
{
    Q_OBJECT
public:
    explicit MSleep(QObject *parent = 0);

    static void msleep(unsigned long msecs) { QThread::msleep(msecs); }
};

#endif // MSLEEP_H
