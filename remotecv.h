#ifndef REMOTECV_H
#define REMOTECV_H

#include <QtGui/QWidget>

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

private:
    Ui::remoteCV *ui;
};

#endif // REMOTECV_H
