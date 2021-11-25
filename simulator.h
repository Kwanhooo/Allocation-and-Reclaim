#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QWidget>
#include <QList>
#include <QThread>
#include <QButtonGroup>
#include <QPushButton>
#include <QMessageBox>

#include <QBitmap>
#include <QPainter>

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include "pcb.h"
#include "partition.h"
#include "partitiondetails.h"

#define PREEMPTIVE_PRIORITY 1195
#define NON_PRIORITY 1196
#define ROUND_ROBIN 1197

namespace Ui {
class Simulator;
}

class Simulator : public QWidget
{
    Q_OBJECT

public:
    explicit Simulator(QWidget *parent = nullptr);
    ~Simulator() override;

    //参数
    const int maxPID = 100000;
    const int maxTime = 30;
    const int maxPriority = 63;
    const int agingTime = 30;

    //内存相关
    const int memorySize = 64;//内存总大小
    const int minNeededLength = 10;//控制随机生成的进程的最小所需内存单位
    const int maxNeededLength = 20;//控制随机生成的进程的最大所需内存单位
    const int systemReservedLength = 16;//系统保留内存大小
    QString unitsOfMeasurement = " MB";

    //内存监视器相关
    const int ORIGIN_X = 1128;
    const int ORIGIN_Y = 42;
    const int TOTAL_HEIGHT = 743;
    const int STANDARD_W = 254;
    QList<QPushButton*>* displayButtonList;

    //参数
    int TIME_SLICE;
    int MAX_PROGRAM_AMOUNT;
    int randomlyEventRate;

    //IO参数
    int USBOccupy;
    int PrinterOccupy;
    int GraphicsOccupy;
    int DiskOccupy;
    int timeScale;
    int IOCount;
    int autoIOGap;

    QTimer *timer;

public slots:
    void getNewPCB(PCB* pcb);
    void setupSimulator(int startMode);

private slots:
    void on_pushButton_custom_clicked();
    void on_pushButton_random_clicked();
    void on_pushButton_manual_clicked();
    void on_pushButton_auto_clicked();
    void on_pushButton_IO_clicked();
    void on_pushButton_suspend_clicked();
    void on_pushButton_IO_All_clicked();
    void on_pushButton_suspend_off_clicked();
    void on_pushButton_auto_stop_clicked();
    void on_spinBox_autoIOGap_valueChanged(int arg1);
    void on_spinBox_IORate_valueChanged(int arg1);
    void on_spinBox_timeSlice_valueChanged(int arg1);
    void on_spinBox_maxProcAmount_valueChanged(const QString &arg1);
    void on_lineEdit_timeScale_textChanged(const QString &arg1);
    void on_pushButton_clicked();
    void on_btn_min_clicked();
    void on_btn_close_clicked() __attribute__((noreturn));
    void on_btn_displayBtn_clicked(Partition* partitionToShow);
    void on_pushButton_reboot_clicked();

protected:
    //鼠标事件重写标示
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private:

    /*
     * 以下为鼠标以及托盘部分
     */

    //鼠标事件相关
    bool m_Drag = false;
    QPoint m_DragPosition;

    //托盘相关
    QSystemTrayIcon* m_systemTray = new QSystemTrayIcon(this);
    QMenu* m_menu;
    QAction* m_action1;
    QAction* m_action2;
    void initSystemTray();

    void activeTray(QSystemTrayIcon::ActivationReason reason);
    void showMenu();
    void showWindow();
    void showMessage();
    QString messageToShow = "Hi There!";
    QString messageTitle = "PowerSimulator";
    const int messageDuration = 1000;

    /*
     * 以下为运行所必要的部分
     */
    Ui::Simulator *ui;
    int startMode;

    PCB* runningProc;//指向正在运行的进程

    /*
     * 容器
     */
    QList<PCB*> readyList;
    QList<PCB*> waitingList;
    QList<PCB*> IOList;
    QList<PCB*> suspendedList;
    QList<PCB*> terminatedList;
    QList<PCB*> backupProcList;
    //分区表
    QList<Partition*> partitionTable;

    //调度方法
    PCB* getTopPriority();//获取最高优先级

    //三种调度方法
    void nonPriorityAction();
    void preemptiveProrityAction();
    void roundRobinAction();

    //通用方法
    void addLog(QString content);//加日志（控制台&UI）
    /*
     * 刷新UI中的展示列表
     */
    void refreshReadyUI();
    void refreshRunningUI();
    void refreshTerminatedUI();
    void resetRunningUI();
    void refreshBackupUI();
    void refreshSuspendedUI();
    void refreshWaitingUI();
    void refreshIOUI();

    //内存相关
    void refreshMemoryUI();
    void initSystemMemory();


    void loadProc();//从后备队列里面载入新的进程

    /*
     * 随机产生IO事件
     * 用在自动化运行
     */
    void ramdomlyEvent();

    void loadIOProc(QString IOType);//从等待队列里面选取进程进行IO

    void IOAction();//完成IO动作
    void IOAll();//一次性IO完

    void automaticRun();//自动运行

    /*
     * 内存分配方法
     * @最先适应算法 first-fit
     */
    int firstFitAction(PCB* pcbToFit);
    void releasePartition(int startingPos);
    int getFreeMemorySize();
    void shrinkAction(int neededLength);

signals:
    //鼠标事件信号
    void mouseButtonMove(QPoint pos);
    void signalMainWindowMove();

    //查看分区详情
    void sendPartitionDetails(Partition* partititon);
};

#endif // SIMULATOR_H
