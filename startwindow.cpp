#include "startwindow.h"
#include "ui_startwindow.h"

#include "simulator.h"

#include <QButtonGroup>

StartWindow::StartWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartWindow)
{
    ui->setupUi(this);

    //实现单选
    QButtonGroup *buttonGround = new QButtonGroup();
    buttonGround->addButton(ui->radioButton_non);
    buttonGround->addButton(ui->radioButton_preemptive);
    buttonGround->addButton(ui->radioButton_rr);
    buttonGround->setExclusive(true);
}

StartWindow::~StartWindow()
{
    delete ui;
}

//将启动模式发送给模拟器主窗口
void StartWindow::on_pushButton_start_clicked()
{
    Simulator* simulator = new Simulator();
    connect(this,SIGNAL(startConfig(int)),simulator,SLOT(setupSimulator(int)));

    if(ui->radioButton_non->isChecked())
        emit startConfig(NON_PRIORITY);

    else if (ui->radioButton_preemptive->isChecked())
        emit startConfig(PREEMPTIVE_PRIORITY);

    else emit startConfig(ROUND_ROBIN);

    this->hide();
    simulator->show();
}

void StartWindow::on_pushButton_exit_clicked()
{
    exit(0);
}
