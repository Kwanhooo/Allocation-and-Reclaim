#ifndef PROCESSCREATE_H
#define PROCESSCREATE_H

#include <QWidget>

#include "pcb.h"
#include "simulator.h"


namespace Ui {
class ProcessCreate;
}

class ProcessCreate : public QWidget
{
    Q_OBJECT

signals:
    void transmitPCB(PCB*);

public:
    explicit ProcessCreate(QWidget *parent = nullptr);
    ~ProcessCreate();

private slots:
    void on_pushButton_exit_clicked();
    void on_pushButton_create_clicked();

private:
    Ui::ProcessCreate *ui;
};

#endif // PROCESSCREATE_H
