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
    void changeEvent(QEvent *e);

private slots:
    void on_cvConnect_clicked();
    void on_cvDisconnect_clicked();
    void onConnectTimeout();

private:
    Ui::remoteCV *ui;
    cvInterface* m_cv;
    QTimer* waitForConnectTimer;
    quint8 timerCount;
};

#endif // REMOTECV_H
