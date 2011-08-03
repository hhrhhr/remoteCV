#include "remotecv.h"
#include "ui_remotecv.h"

remoteCV::remoteCV(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::remoteCV)
{
    ui->setupUi(this);

    statusTimer = new QTimer();
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timerCount = 0;
    cvstate = CVInterface::cvDisconnected;

    m_cv = new CVInterface(this);
    connect(m_cv, SIGNAL(cvStateChanged(CVInterface::cvState)),
            this, SLOT(oncvStateChanged(CVInterface::cvState)));
    connect(m_cv, SIGNAL(cvStateError(QString)),
            this, SLOT(oncvStateError(QString)));
}

remoteCV::~remoteCV()
{
    delete ui;
    disconnect(m_cv, SIGNAL(cvStateChanged(CVInterface::cvState)),
               this, SLOT(oncvStateChanged(CVInterface::cvState)));
    delete m_cv;
}

// protected

// private slots

void remoteCV::on_cvConnect_clicked()
{
    ui->cvConnect->setDisabled(TRUE);
    statusTimer->start(1000);
    m_cv->connectToCV(ui->cvHost->text(), ui->cvPort->value());
}

void remoteCV::on_cvDisconnect_clicked()
{
    m_cv->disconnectFromCV();
    statusTimer->stop();
    ui->cvLastError->setText("no error");
    ui->cvDisconnect->setDisabled(TRUE);
    ui->cvConnect->setEnabled(TRUE);
    ui->cvStatus->setText("disconnected");
    cvstate = CVInterface::cvDisconnected;
}

void remoteCV::onTimeout()
{
    if (cvstate == CVInterface::cvConnecting
        || cvstate == CVInterface::cvHostFound
        || cvstate == CVInterface::cvError) {
        QString status = ui->cvStatus->text();
        if (timerCount < 4) {
            timerCount++;
            status += ".";
        } else {
            timerCount = 0;
            status = status.replace(".", "");
            ui->cvDisconnect->setEnabled(true);
            if (cvstate == CVInterface::cvError)
                on_cvConnect_clicked();
        }
        ui->cvStatus->setText(status);
    }
}

void remoteCV::oncvStateChanged(CVInterface::cvState state)
{
    cvstate = state;
    switch (state) {
        case CVInterface::cvConnecting :
            ui->cvStatus->setText("connecting");
            break;
        case CVInterface::cvHostFound :
            ui->cvDisconnect->setDisabled(true);
            break;
        case CVInterface::cvConnected :
            ui->cvStatus->setText("connected");
            ui->cvDisconnect->setEnabled(true);
            break;
        case CVInterface::cvDisconnecting :
            ui->cvStatus->setText("disconnecting");
            break;
        case CVInterface::cvDisconnected :
            ui->cvStatus->setText("disconnected");
            ui->cvLastError->setText("no error");
            ui->cvDisconnect->setDisabled(TRUE);
            ui->cvConnect->setEnabled(TRUE);
            break;
        case CVInterface::cvError :
//            ui->cvStatus->setText("error");
            ui->cvDisconnect->setEnabled(true);
            break;
        case CVInterface::cvUnknown :
            ui->cvStatus->setText("unknown");
            break;
        default :
            ui->cvDisconnect->setEnabled(true);
    }
}

void remoteCV::oncvStateError(QString socketError)
{
    ui->cvLastError->setText(socketError);
}

void remoteCV::on_getTelemetry_clicked()
{
    m_cv->sendCommand("getTelemetry-2\n");
}

void remoteCV::on_requestControl_clicked()
{
    m_cv->sendCommand("requestControl-2\n");
}

void remoteCV::on_setControl_clicked()
{
    m_cv->sendCommand("setControls-2\n");
}

void remoteCV::on_releaseControl_clicked()
{
    m_cv->sendCommand("releaseControl-2\n");
}
