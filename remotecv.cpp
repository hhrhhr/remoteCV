#include "remotecv.h"
#include "ui_remotecv.h"

RemoteCV::RemoteCV(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::remoteCV)
{
    qDebug() << "RemoteCV";
    ui->setupUi(this);

    statusTimer = new QTimer();
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timerCount = 0;
    cvstate = CVInterface::cvDisconnected;

    m_cv = new CVInterface(this);
    connect(m_cv, SIGNAL(cvStateChanged(CVInterface::cvState, QString)),
            this, SLOT(oncvStateChanged(CVInterface::cvState, QString)));
    connect(m_cv, SIGNAL(processOutput(QString, QString)), this, SLOT(slotProcessOutput(QString, QString)));

    screenUpdate = new QTimer();
    connect(screenUpdate, SIGNAL(timeout()), this, SLOT(onScreenUpdate()));
    textUpdate = new QTime();
}

RemoteCV::~RemoteCV()
{
    qDebug() << "~RemoteCV";
}

// private slots

void RemoteCV::on_cvConnect_clicked()
{
    ui->cvConnect->setDisabled(TRUE);
    statusTimer->start(1000);
    screenUpdate->start(ui->cvRefresh_2->value());
    textUpdate->start();
    m_cv->connectToCV(ui->cvHost->text(), ui->cvPort->value());
}

void RemoteCV::on_cvDisconnect_clicked()
{
    m_cv->disconnectFromCV();
    statusTimer->stop();
    screenUpdate->stop();

    ui->cvLastError->setText("no error");
    ui->cvDisconnect->setDisabled(TRUE);
    ui->cvConnect->setEnabled(TRUE);
    ui->cvStatus->setText("disconnected");
    cvstate = CVInterface::cvDisconnected;
}

void RemoteCV::onTimeout()
{
    if (cvstate == CVInterface::cvConnecting || cvstate == CVInterface::cvHostFound
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

void RemoteCV::oncvStateChanged(CVInterface::cvState state, QString error)
{
    ui->cvLastError->setText(error);
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

void RemoteCV::slotProcessOutput(QString txtRaw, QString txtParsed)
{
    if (txtRaw > "")
        ui->consoleRaw->setPlainText(txtRaw);
    if (txtParsed > "")
        ui->consoleParsed->setPlainText(txtParsed);
}

void RemoteCV::onScreenUpdate()
{
    m_cv->sendCommand("getTelemetry-2\n");
    Attitude* a = m_cv->m_calc->att;

    switch (ui->tabWidget->currentIndex()) {
        case 0:
            if (textUpdate->elapsed() > 333) {
                //

                textUpdate->restart();
            }
            break;
        case 1:
            // refresh of text label - 3Hz
            if (textUpdate->elapsed() > 333) {
                ui->xPos->setText(QString::number(a->position.x(), 'f', 2) + "m");
                ui->yPos->setText(QString::number(a->position.y(), 'f', 2) + "m");
                ui->zPos->setText(QString::number(a->position.z(), 'f', 2) + "m");
                ui->xSpeed->setText(QString::number(a->speedNED.x(), 'f', 2) + "m/s");
                ui->ySpeed->setText(QString::number(a->speedNED.y(), 'f', 2) + "m/s");
                ui->zSpeed->setText(QString::number(a->speedNED.z(), 'f', 2) + "m/s");
                ui->lbSimTimer->setText(QString::number(a->time/1000) + "sec");
                ui->lbAirSpeed->setText(QString::number(a->airspeed, 'f', 2) + "m/s");
                ui->lbGroundSpeed->setText(QString::number(a->groundspeed, 'f', 2) + "m/s");
                textUpdate->restart();
            }

            ui->roll->setValue(a->attitude.x());
            ui->pitch->setValue(a->attitude.y());
            ui->yaw->setValue(a->attitude.z());

//            ui->roll_2->setValue(a->attitude2.x());
//            ui->pitch_2->setValue(a->attitude2.y());
//            ui->yaw_2->setValue(a->attitude2.z());

            ui->xRate->setValue(a->gyro.x());
            ui->yRate->setValue(a->gyro.y());
            ui->zRate->setValue(a->gyro.z());

//            ui->xSpeedRaw->setValue(a->speedNED.x()*10);
//            ui->ySpeedRaw->setValue(a->speedNED.y()*10);
//            ui->zSpeedRaw->setValue(a->speedNED.z()*10);

//            ui->xSpeedRaw_2->setValue(a->speedNED2.x()*10);
//            ui->ySpeedRaw_2->setValue(a->speedNED2.y()*10);
//            ui->zSpeedRaw_2->setValue(a->speedNED2.z()*10);

            ui->xAccel->setValue(a->accel.x()*10);
            ui->yAccel->setValue(a->accel.y()*10);
            ui->zAccel->setValue(a->accel.z()*10);

//            ui->xAccel_2->setValue(a->accel2.x()*10);
//            ui->yAccel_2->setValue(a->accel2.y()*10);
//            ui->zAccel_2->setValue(a->accel2.z()*10);
            break;
        case 2:
            ui->ch1->setValue(a->controls[0]);
            ui->ch2->setValue(a->controls[1]);
            ui->ch3->setValue(a->controls[2]);
            ui->ch4->setValue(a->controls[3]);
            ui->ch5->setValue(a->controls[4]);
            ui->ch6->setValue(a->controls[5]);
            break;
    }
}

void RemoteCV::on_getTelemetry_clicked()
{
    m_cv->sendCommand("getTelemetry-2\n");
    emit needTelemetry(1);
}

void RemoteCV::on_getRawTelemetry_clicked()
{
    m_cv->sendCommand("getTelemetry-2\n");
    emit needTelemetry(2);
}

void RemoteCV::on_getParsedTelemetry_clicked()
{
    m_cv->sendCommand("getTelemetry-2\n");
    emit needTelemetry(3);
}

void RemoteCV::on_requestControl_clicked()
{
    m_cv->sendCommand("requestControl-2\n");
}

void RemoteCV::on_setControl_clicked()
{
    m_cv->sendCommand("setControls-2\n");
}

void RemoteCV::on_releaseControl_clicked()
{
    m_cv->sendCommand("releaseControl-2\n");
}

void RemoteCV::on_cvRefresh_valueChanged(int interval)
{
    screenUpdate->setInterval(interval);
}

void RemoteCV::on_cvRefresh_2_valueChanged(int interval)
{
    screenUpdate->setInterval(interval);
}

void RemoteCV::on_radioButton_toggled(bool checked)
{
    if (checked) {
        m_cv->m_calc->accelUseSpeed = TRUE;
    } else {
        m_cv->m_calc->accelUseSpeed = FALSE;
    }
}
