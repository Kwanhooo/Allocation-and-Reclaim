#include "partitiondetails.h"
#include "ui_partitiondetails.h"

#include <QDebug>

PartitionDetails::PartitionDetails(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PartitionDetails)
{
    ui->setupUi(this);
    this->setWindowTitle("详细信息 - PowerSimulator");
    this->setFixedSize(this->width(),this->height());
    this->setWindowFlag(Qt::FramelessWindowHint);
    ui->titleBarGroup->setAlignment(Qt::AlignRight);
}

PartitionDetails::~PartitionDetails()
{
    delete ui;
}

void PartitionDetails::setDisplayPartition(Partition *partitionToShow)
{
    this->partition = partitionToShow;
    setupDisplayData();
}

/*
 * 以下代码段为隐藏标题栏之后，重写的鼠标事件
 */
void PartitionDetails::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_Drag = true;
        m_DragPosition = event->globalPos() - this->pos();
        event->accept();
    }
    QWidget::mousePressEvent(event);
}

void PartitionDetails::setupDisplayData()
{
    ui->lineEdit_startingPos->setEnabled(false);
    ui->lineEdit_length->setEnabled(false);
    ui->lineEdit_partitionStatus->setEnabled(false);
    ui->lineEdit_PID->setEnabled(false);
    ui->lineEdit_neededMemory->setEnabled(false);
    ui->lineEdit_neededTime->setEnabled(false);
    ui->lineEdit_calTime->setEnabled(false);
    ui->lineEdit_procStatus->setEnabled(false);
    ui->lineEdit_priority->setEnabled(false);

    ui->lineEdit_startingPos->setText(QString::number(partition->getStart()).append(" --> ").append(QString::number(partition->getStart()+partition->getLength())));
    ui->lineEdit_length->setText(QString::number(partition->getLength()));

    QString statusEmoji;
    if(partition->getStatus() == 0)
        statusEmoji = "🔑 空闲";
    else
        statusEmoji = "🔒 占用";

    ui->lineEdit_partitionStatus->setText(statusEmoji);

    if(partition->associatedPCB == nullptr)
    {
        this->setFixedHeight(this->height() - 225);
        ui->groupBox_proc->hide();
    }
    else
    {
        ui->lineEdit_PID->setText(QString::number(partition->associatedPCB->getPid()));
        ui->lineEdit_neededMemory->setText(QString::number(partition->associatedPCB->getNeededLength()));
        ui->lineEdit_neededTime->setText(QString::number(partition->associatedPCB->getNeededTime()));
        ui->lineEdit_calTime->setText(QString::number(partition->associatedPCB->getCalUseTime()));

        /*
         * 0 -> backup
         * 1 -> ready
         * 2 -> running
         * 3 -> waiting
         * 4 -> IO
         * 5 -> suspended
         * 6 -> terminated
         */
        QString procStatusEmoji;
        switch (partition->associatedPCB->getStatus())
        {
            case 1:procStatusEmoji = "🆗 就绪";break;
            case 2:procStatusEmoji = "🚀 运行";break;
            case 3:procStatusEmoji = "🚥 等待";break;
            case 4:procStatusEmoji = "🖨️ IO";break;
            default:procStatusEmoji = "UNKNOWN";
        }

        ui->lineEdit_procStatus->setText(procStatusEmoji);

        if(partition->associatedPCB->getPriority() == -1)
        {
            this->setFixedHeight(this->height()-40);
            ui->groupBox_proc->setGeometry(ui->groupBox_proc->x(),ui->groupBox_proc->y(),ui->groupBox_proc->width(),ui->groupBox_proc->height()-30);
            ui->lineEdit_priority->hide();
            ui->label_priority->hide();
        }
        else
        {
            ui->lineEdit_priority->setText(QString::number(partition->associatedPCB->getPriority()));
        }
    }
}


void PartitionDetails::mouseMoveEvent(QMouseEvent *event)
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


void PartitionDetails::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_Drag = false;
    QWidget::mouseReleaseEvent(event);
}

//最小化这个窗口
void PartitionDetails::on_btn_min_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

//关闭这个窗口
void PartitionDetails::on_btn_close_clicked()
{
    this->close();
    delete this;
}
