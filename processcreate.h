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
    ~ProcessCreate() override;

protected:
    //鼠标事件重写标示
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private slots:
    void on_pushButton_exit_clicked();
    void on_pushButton_create_clicked();

    void on_btn_min_clicked();

    void on_btn_close_clicked();

private:
    Ui::ProcessCreate *ui;

    //鼠标事件相关变量
    bool m_Drag = false;
    QPoint m_DragPosition;

signals:
    //鼠标事件信号
    void mouseButtonMove(QPoint pos);
    void signalMainWindowMove();
};

#endif // PROCESSCREATE_H
