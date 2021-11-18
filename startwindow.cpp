#include "startwindow.h"
#include "ui_startwindow.h"

#include "simulator.h"

StartWindow::StartWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("启动 - PowerSimulator");
    this->setFixedSize(this->width(),this->height());
    this->setWindowFlag(Qt::FramelessWindowHint);
    ui->titleBarGroup->setAlignment(Qt::AlignRight);

    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    p.setPen(Qt::transparent);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect(), 15, 15);
    setMask(bmp);

    //实现单选
    QButtonGroup *buttonGround = new QButtonGroup();
    buttonGround->addButton(ui->radioButton_non);
    buttonGround->addButton(ui->radioButton_preemptive);
    buttonGround->addButton(ui->radioButton_rr);
    buttonGround->setExclusive(true);

    //默认非抢占式优先权调度
    ui->radioButton_non->setChecked(true);
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


/*
 * 以下代码段为隐藏标题栏之后，重写的鼠标事件
 */
void StartWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_Drag = true;
        m_DragPosition = event->globalPos() - this->pos();
        event->accept();
    }
    QWidget::mousePressEvent(event);
}


void StartWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_Drag && (event->buttons() && static_cast<bool>(Qt::LeftButton)))
    {
        move(event->globalPos() - m_DragPosition);
        event->accept();
        emit mouseButtonMove(event->globalPos() - m_DragPosition);
        emit signalMainWindowMove();
    }
    QWidget::mouseMoveEvent(event);
}


void StartWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_Drag = false;
    QWidget::mouseReleaseEvent(event);
}


//最小化这个窗口
void StartWindow::on_btn_min_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

//关闭这个窗口
void StartWindow::on_btn_close_clicked()
{
    exit(0);
}
