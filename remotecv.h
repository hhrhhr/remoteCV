#ifndef REMOTECV_H
#define REMOTECV_H

#include <QtGui/QWidget>
#include <QDebug>

#include <cvinterface.h>
#include <QTimer>

namespace Ui {
    class remoteCV;
}

class remoteCV : public QWidget
{
    Q_OBJECT

public:
    explicit remoteCV(QWidget *parent = 0);
    ~remoteCV();

protected:

private:
    Ui::remoteCV *ui;
    CVInterface *m_cv;
    QTimer *statusTimer;
    quint8 timerCount;
    CVInterface::cvState cvstate;

private slots:
    void on_cvConnect_clicked();
    void on_cvDisconnect_clicked();
    void onTimeout();
    void oncvStateChanged(CVInterface::cvState state);
    void oncvStateError(QString socketError);

    void on_getTelemetry_clicked();
    void on_requestControl_clicked();
    void on_setControl_clicked();
    void on_releaseControl_clicked();
};

#endif // REMOTECV_H
