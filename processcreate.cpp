#include "processcreate.h"
#include "ui_processcreate.h"
#include <QDebug>

ProcessCreate::ProcessCreate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProcessCreate)
{
    ui->setupUi(this);
    this->setWindowTitle("创建进程 - PowerSimulator");
    this->setWindowFlag(Qt::FramelessWindowHint);
    ui->titleBarGroup->setAlignment(Qt::AlignRight);
}

ProcessCreate::~ProcessCreate()
{
    delete ui;
}

void ProcessCreate::on_pushButton_exit_clicked()
{
    this->close();
    delete this;
}

void ProcessCreate::on_pushButton_create_clicked()
{
    qDebug()<< ui->lineEdit_pid->text().toInt();
    PCB* newPCB = new PCB(ui->lineEdit_pid->text().toInt(),ui->lineEdit_neededtime->text().toInt(),ui->lineEdit_priority->text().toInt(),ui->lineEdit_neededLength->text().toInt());
    emit transmitPCB(newPCB);
}

/*
 * 以下代码段为隐藏标题栏之后，重写的鼠标事件
 */
void ProcessCreate::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_Drag = true;
        m_DragPosition = event->globalPos() - this->pos();
        event->accept();
    }
    QWidget::mousePressEvent(event);
}


void ProcessCreate::mouseMoveEvent(QMouseEvent *event)
{
    if (m_Drag && (event->buttons() && Qt::LeftButton))
    {
        move(event->globalPos() - m_DragPosition);
        event->accept();
        emit mouseButtonMove(event->globalPos() - m_DragPosition);
        emit signalMainWindowMove();
    }
    QWidget::mouseMoveEvent(event);
}


void ProcessCreate::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_Drag = false;
    QWidget::mouseReleaseEvent(event);
}

//最小化这个窗口
void ProcessCreate::on_btn_min_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

//关闭这个窗口
void ProcessCreate::on_btn_close_clicked()
{
    this->close();
    delete this;
}

