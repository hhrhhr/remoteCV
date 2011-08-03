#include "remotecv.h"
#include "ui_remotecv.h"

remoteCV::remoteCV(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::remoteCV)
{
    ui->setupUi(this);
}

remoteCV::~remoteCV()
{
    delete ui;
}

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
