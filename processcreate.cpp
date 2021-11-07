#include "processcreate.h"
#include "ui_processcreate.h"
#include <QDebug>

ProcessCreate::ProcessCreate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProcessCreate)
{
    ui->setupUi(this);
    this->setWindowTitle("创建进程");
}

ProcessCreate::~ProcessCreate()
{
    delete ui;
}

void ProcessCreate::on_pushButton_exit_clicked()
{
    this->close();
}

void ProcessCreate::on_pushButton_create_clicked()
{
    qDebug()<< ui->lineEdit_pid->text().toInt();
    PCB* newPCB = new PCB(ui->lineEdit_pid->text().toInt(),ui->lineEdit_neededtime->text().toInt(),ui->lineEdit_priority->text().toInt());
    emit transmitPCB(newPCB);
}
