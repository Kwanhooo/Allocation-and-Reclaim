#ifndef PARTITIONDETAILS_H
#define PARTITIONDETAILS_H

#include <QMouseEvent>
#include <QWidget>

#include "partition.h"

namespace Ui {
class PartitionDetails;
}

class PartitionDetails : public QWidget
{
    Q_OBJECT

public:
    explicit PartitionDetails(QWidget *parent = nullptr);
    ~PartitionDetails() override;

public slots:
    void setDisplayPartition(Partition* partitionToShow);

protected:
    //鼠标事件重写标示
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::PartitionDetails *ui;

    Partition* partition;

    //鼠标事件相关变量
    bool m_Drag = false;
    QPoint m_DragPosition;

    void setupDisplayData();

signals:
    //鼠标事件信号
    void mouseButtonMove(QPoint pos);
    void signalMainWindowMove();
private slots:
    void on_btn_min_clicked();
    void on_btn_close_clicked();
};

#endif // PARTITIONDETAILS_H
