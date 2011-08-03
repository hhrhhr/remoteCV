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
    cvstate = cvInterface::cvDisconnected;

    m_cv = new cvInterface(this);
    connect(m_cv, SIGNAL(cvStateChanged(cvInterface::cvState)),
            this, SLOT(oncvStateChanged(cvInterface::cvState)));
}

remoteCV::~remoteCV()
{
    delete ui;
    disconnect(m_cv, SIGNAL(cvStateChanged(cvInterface::cvState)),
               this, SLOT(oncvStateChanged(cvInterface::cvState)));
    delete m_cv;
}

// protected

void remoteCV::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

// private slots

void remoteCV::on_cvConnect_clicked()
{
    ui->cvConnect->setDisabled(true);
    ui->cvDisconnect->setEnabled(true);
    statusTimer->start(1000);

    m_cv->connectToCV(ui->cvHost->text(), ui->cvPort->value());
}

void remoteCV::on_cvDisconnect_clicked()
{
    ui->cvDisconnect->setDisabled(true);
    ui->cvConnect->setEnabled(true);

    m_cv->disconnectFromCV();
}

void remoteCV::onTimeout()
{
    if (cvstate == cvInterface::cvConnecting
        || cvstate == cvInterface::cvDisconnecting
        || cvstate == cvInterface::cvError) {
        QString status = ui->cvStatus->text();
        if (timerCount < 5) {
            timerCount++;
            status += ".";
        } else {
            timerCount = 0;
            status = status.replace(".", "");
            if (cvstate == cvInterface::cvDisconnecting) {
                cvstate = cvInterface::cvDisconnected;
                status = "disconnected";
                statusTimer->stop();
            } else if (cvstate == cvInterface::cvError) {
                on_cvConnect_clicked();
            }
        }
        ui->cvStatus->setText(status);
    }
}

void remoteCV::oncvStateChanged(cvInterface::cvState state)
{
    cvstate = state;
    switch (state) {
        case cvInterface::cvConnecting :
            ui->cvStatus->setText("connecting");
            break;
        case cvInterface::cvConnected :
            ui->cvStatus->setText("connected");
            break;
        case cvInterface::cvDisconnecting :
            ui->cvStatus->setText("disconnecting");
            break;
        case cvInterface::cvDisconnected :
            ui->cvStatus->setText("disconnected");
            break;
        case cvInterface::cvError :
//            ui->cvStatus->setText("error");
            break;
        case cvInterface::cvUnknown :
            ui->cvStatus->setText("unknown");
            break;
    }
}

