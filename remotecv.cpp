#include "remotecv.h"
#include "ui_remotecv.h"

remoteCV::remoteCV(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::remoteCV)
{
    ui->setupUi(this);

    waitForConnectTimer = new QTimer();
    connect(waitForConnectTimer, SIGNAL(timeout()), this, SLOT(onConnectTimeout()));
    timerCount = 0;
}

remoteCV::~remoteCV()
{
    delete ui;
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
    ui->cvStatus->setText("conecting");
    ui->cvConnect->setDisabled(true);
    ui->cvDisconnect->setEnabled(true);
    m_cv = new cvInterface(this);
    waitForConnectTimer->start(500);
    m_cv->connectToCV(ui->cvHost->text(), ui->cvPort->value());
}

void remoteCV::on_cvDisconnect_clicked()
{
    ui->cvStatus->setText("disconecting");
    ui->cvDisconnect->setDisabled(true);
    ui->cvConnect->setEnabled(true);
    waitForConnectTimer->start(500);
    m_cv->disconnectFromCV();
    m_cv->deleteLater();
}

void remoteCV::onConnectTimeout()
{
    QString status = ui->cvStatus->text();
    timerCount++;
    if (timerCount % 5) {
        status += ".";
    } else {
        status = status.replace(".", "");
    }
    ui->cvStatus->setText(status);
}


