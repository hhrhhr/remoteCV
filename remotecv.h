#ifndef REMOTECV_H
#define REMOTECV_H

#include <QtGui/QWidget>
#include <QDebug>

#include <cvinterface.h>
#include <QTimer>
#include <QTime>

namespace Ui {
    class remoteCV;
}

class RemoteCV : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteCV(QWidget *parent = 0);
    ~RemoteCV();

signals:
    void needTelemetry(quint8);

private:
    Ui::remoteCV* ui;
    CVInterface* m_cv;
    QTimer* statusTimer;
    QTimer* screenUpdate;
    QTime* textUpdate;
    quint8 timerCount;
    CVInterface::cvState cvstate;

private slots:
    void on_cvConnect_clicked();
    void on_cvDisconnect_clicked();
    void onTimeout();
    void oncvStateChanged(CVInterface::cvState state, QString error);
    void slotProcessOutput(QString txtRaw, QString txtParsed);

    void onScreenUpdate();

    void on_getTelemetry_clicked();
    void on_getRawTelemetry_clicked();
    void on_getParsedTelemetry_clicked();
    void on_requestControl_clicked();
    void on_setControl_clicked();
    void on_releaseControl_clicked();
    void on_cvRefresh_valueChanged(int interval);
    void on_cvRefresh_2_valueChanged(int interval);
    void on_radioButton_toggled(bool checked);
};

#endif // REMOTECV_H
