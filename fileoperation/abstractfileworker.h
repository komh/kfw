#ifndef ABSTRACTFILEWORKER_H
#define ABSTRACTFILEWORKER_H

#include <QObject>

class AbstractFileWorker : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFileWorker(const QString& source, const QString& dest,
                                QObject *parent = 0);

    void cancel() { _canceled = true; }
    bool wasCanceled() const { return _canceled; }

    const QString& source() const { return _source; }
    const QString& dest() const { return _dest; }

    bool result() const { return _result; }

signals:
    void valueChanged(int value);

public slots:
    void perform();

protected:
    virtual void performWork() = 0;

    void setResult(bool result) { _result = result; }

private:
    QString _source;
    QString _dest;

    bool _canceled;
    bool _result;
};

#endif // ABSTRACTFILEWORKER_H
