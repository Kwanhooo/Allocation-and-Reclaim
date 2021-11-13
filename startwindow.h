#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QMainWindow>

#define PREEMPTIVE_PRIORITY 1195
#define NON_PRIORITY 1196
#define ROUND_ROBIN 1197

namespace Ui {
class StartWindow;
}

class StartWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void startConfig(int startMode);

public:
    explicit StartWindow(QWidget *parent = nullptr);
    ~StartWindow();

protected:
    //鼠标事件重写标示
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_exit_clicked();

    void on_btn_min_clicked();

    void on_btn_close_clicked();

private:
    Ui::StartWindow *ui;

    //鼠标事件相关变量
    bool m_Drag = false;
    QPoint m_DragPosition;

signals:
    //鼠标事件信号
    void mouseButtonMove(QPoint pos);
    void signalMainWindowMove();

};

#endif // STARTWINDOW_H
