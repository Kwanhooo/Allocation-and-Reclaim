#include "simulator.h"
#include "ui_simulator.h"
#include "processcreate.h"

#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QStringListModel>
#include <QStandardItemModel>
#include "windows.h"

Simulator::Simulator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Simulator)
{
    ui->setupUi(this);
    this->setWindowTitle("PowerSimulator");

    this->runningProc = nullptr;
    this->USBOccupy = 0;
    this->PrinterOccupy = 0;
    this->DiskOccupy = 0;
    this->GraphicsOccupy = 0;
    this->IOCount = 0;

    TIME_SLICE = 10;
    MAX_PROGRAM_AMOUNT = 8;
    randomlyEventRate = 5;
    autoIOGap = 30;

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Simulator::automaticRun);
    this->timeScale = 100;

    ui->lineEdit_timeScale->setText(QString::number(timeScale));
    ui->spinBox_IORate->setValue(this->randomlyEventRate);
    ui->spinBox_timeSlice->setValue(this->TIME_SLICE);
    ui->spinBox_maxProcAmount->setValue(this->MAX_PROGRAM_AMOUNT);
    ui->spinBox_autoIOGap->setValue(this->autoIOGap);
    ui->label_rotation->setVisible(false);
    ui->label_IO_icon->setVisible(false);

    this->displayButtonList = new QList<QPushButton*>();

    //初始化内存分区表
    this->partitionTable.append(new Partition(0,this->memorySize,0));
    this->refreshMemoryUI();

    testInt = 1;
}

//接受启动模式的槽函数
void Simulator::setupSimulator(int startMode)
{
    this->startMode = startMode;
    QString startModeStr;
    switch (startMode)
    {
    case 1195:startModeStr = "PRRMPTIVE PRIORITY";break;
    case 1196:startModeStr = "NON-PRRMPTIVE PRIORITY";break;
    case 1197:startModeStr = "ROUND-ROBIN";break;
    default:startModeStr = "ERROR";
    }
    addLog(QString("Simulator已以\""+startModeStr+"\"模式加载启动"));
}

Simulator::~Simulator()
{
    delete ui;
}

//按照优先级字段排序
bool comparePriority(PCB* pcb_a,PCB* pcb_b)
{
    if(pcb_a->getPriority() < pcb_b->getPriority())
        return true;
    else
        return false;
}

//刷新ready队列的UI
void Simulator::refreshReadyUI()
{
    //优先权算法的话就按照优先级排一下序
    if(this->startMode == PREEMPTIVE_PRIORITY || this->startMode == NON_PRIORITY)
        std::sort(readyList.begin(),readyList.end(),comparePriority);

    QStringList readyStringList;
    for (int i = 0;i < this->readyList.length(); i++)
    {
        if(this->startMode == ROUND_ROBIN)
            readyStringList<<"PID:"+QString::number(readyList.at(i)->getPid())+"\n"+
                             "剩余运行时长:"+QString::number(readyList.at(i)->getCalUseTime())+"\n"+
                             "占用内存:"+QString::number(readyList.at(i)->getNeededLength())+"\n";
        else
            readyStringList<<"PID:"+QString::number(readyList.at(i)->getPid())+"\n"+
                             "剩余运行时长:"+QString::number(readyList.at(i)->getCalUseTime())+"\n"
                                                                                         "优先级:"+QString::number(readyList.at(i)->getPriority())+"\n"+
                             "占用内存:"+QString::number(readyList.at(i)->getNeededLength())+"\n";
    }
    QStringListModel* readyStringListModel = new QStringListModel(readyStringList);
    ui->listView_ready->setModel(readyStringListModel);
    ui->label_readyLength->setText(QString::number(readyList.length()));
}

void Simulator::refreshRunningUI()
{
    QStringList runningStringList;
    if(startMode == ROUND_ROBIN)
        runningStringList<<"PID:"+QString::number(runningProc->getPid())+"\n"+
                           "剩余时长:"+QString::number(runningProc->getCalUseTime())+"\n"
                                                                                 "剩余时间片:"+QString::number(TIME_SLICE-runningProc->getUsedTimeSlice())+"\n"+
                           "占用内存:"+QString::number(runningProc->getNeededLength())+"\n";
    else
        runningStringList<<"PID:"+QString::number(runningProc->getPid())+"\n"+
                           "剩余时长:"+QString::number(runningProc->getCalUseTime())+"\n"
                                                                                 "优先级:"+QString::number(runningProc->getPriority())+"\n"+
                           "占用内存:"+QString::number(runningProc->getNeededLength())+"\n";
    QStringListModel* runningStringListModel = new QStringListModel(runningStringList);
    ui->listView_running->setModel(runningStringListModel);
    if(runningProc == nullptr)
        ui->label_runningLength->setText(QString::number(0));
    else
        ui->label_runningLength->setText(QString::number(1));
}

void Simulator::refreshTerminatedUI()
{
    if(this->startMode == PREEMPTIVE_PRIORITY || this->startMode == NON_PRIORITY)
        std::sort(terminatedList.begin(),terminatedList.end(),comparePriority);

    QStringList terminatedStringList;
    for (int i = 0;i < this->terminatedList.length(); i++)
    {
        if(startMode == ROUND_ROBIN)
            terminatedStringList<<"PID:"+QString::number(terminatedList.at(i)->getPid())+"\n"+
                                  "总消耗时长:"+QString::number(terminatedList.at(i)->getNeededTime())+"\n";
        else
            terminatedStringList<<"PID:"+QString::number(terminatedList.at(i)->getPid())+"\n"+
                                  "总消耗时长:"+QString::number(terminatedList.at(i)->getNeededTime())+"\n"
                                                                                                  "优先级:"+QString::number(terminatedList.at(i)->getPriority())+"\n";
    }
    QStringListModel* terminatedStringListModel = new QStringListModel(terminatedStringList);
    ui->listView_terminated->setModel(terminatedStringListModel);
    ui->label_terminatedLength->setText(QString::number(terminatedList.length()));
}

void Simulator::resetRunningUI()
{
    QStringListModel* clearModel = new QStringListModel();
    ui->listView_running->setModel(clearModel);
    ui->label_runningLength->setText(QString::number(0));
}

void Simulator::refreshBackupUI()
{
    QStringList backupProcStringList;
    for (int i = 0;i < this->backupProcList.length(); i++)
    {
        if(startMode == ROUND_ROBIN)
            backupProcStringList<<"PID:"+QString::number(backupProcList.at(i)->getPid())+"\n"+
                                  "所需时长:"+QString::number(backupProcList.at(i)->getNeededTime())+"\n"+
                                  "所需内存:"+QString::number(backupProcList.at(i)->getNeededLength())+"\n";
        else
            backupProcStringList<<"PID:"+QString::number(backupProcList.at(i)->getPid())+"\n"+
                                  "所需时长:"+QString::number(backupProcList.at(i)->getNeededTime())+"\n"
                                                                                                 "优先级:"+QString::number(backupProcList.at(i)->getPriority())+"\n"+
                                  "所需内存:"+QString::number(backupProcList.at(i)->getNeededLength())+"\n";
    }
    QStringListModel* backupProcStringListModel = new QStringListModel(backupProcStringList);
    ui->listView_backup->setModel(backupProcStringListModel);
    ui->label_backLength->setText(QString::number(backupProcList.length()));
}

//从后备队列里面装载进程
void Simulator::loadProc()
{
    qDebug()<<"backupProcList.isEmpty() == "<<(backupProcList.isEmpty())<<endl;
    if(backupProcList.isEmpty())
        return;
    if(readyList.length() >= MAX_PROGRAM_AMOUNT)
        return;
    PCB* pcbToLoad = this->backupProcList.takeFirst();//取出第一个

    int startingPos = this->firstFitAction(pcbToLoad->getNeededLength());

    if(startingPos == -1)//放不下
    {
        addLog(QString("装载进程失败：PID = ").append(QString::number(pcbToLoad->getPid()))
               .append("  ->内存不足\n\t").append("所需内存:").append(QString::number(pcbToLoad->getNeededLength())));
        this->backupProcList.append(pcbToLoad);
        this->refreshBackupUI();
    }
    else//放得下
    {
        pcbToLoad->setStartingPos(startingPos);
        this->readyList.append(pcbToLoad);
        addLog(QString("装载进程：PID = ").append(QString::number(pcbToLoad->getPid())).append("至Ready队列   ")
               .append("内存起址:").append(QString::number(pcbToLoad->getStartingPos())));
        this->refreshReadyUI();
        this->refreshBackupUI();
        this->refreshMemoryUI();
    }
}

void Simulator::refreshSuspendedUI()
{
    QStringList suspendedStringList;
    for (int i = 0;i < this->suspendedList.length(); i++)
    {
        if(startMode == ROUND_ROBIN)
            suspendedStringList<<"PID:"+QString::number(suspendedList.at(i)->getPid())+"\n"+
                                 "所需时长:"+QString::number(suspendedList.at(i)->getCalUseTime())+"\n";
        else
            suspendedStringList<<"PID:"+QString::number(suspendedList.at(i)->getPid())+"\n"+
                                 "所需时长:"+QString::number(suspendedList.at(i)->getCalUseTime())+"\n"
                                                                                               "优先级:"+QString::number(suspendedList.at(i)->getPriority())+"\n";
    }
    QStringListModel* suspendedStringListModel = new QStringListModel(suspendedStringList);
    ui->listView_suspended->setModel(suspendedStringListModel);
    ui->label_suspendedLength->setText(QString::number(suspendedList.length()));
}

void Simulator::ramdomlyEvent()
{
    if(runningProc == nullptr)
        return;
    QTime t;
    t=QTime::currentTime();
    qsrand(QTime::currentTime().msec()*qrand()*qrand()*qrand()*qrand()*qrand()*qrand());
    int n = qrand()%100 + 1;
    qDebug()<<"是不是生成了随机事件呢?  "<<n<<"  "<<(n<=this->randomlyEventRate)<<endl;
    bool isGenerate = n <= this->randomlyEventRate?true:false;
    if(!isGenerate)
        return;
    //生成事件类型
    int eventType = qrand()%4 + 1;//[1,4]
    qDebug()<<"是什么随机事件呢?  "<<eventType<<endl;
    switch (eventType)
    {
    case 1:runningProc->eventType = "USB";
        addLog(QString("PID = ").append(QString::number(runningProc->getPid())).append(" -> 接受到IO申请\n\t").append("申请IO资源:").append(runningProc->eventType));
        if(USBOccupy < 3)
        {
            this->IOList.append(runningProc);
            refreshIOUI();
            this->USBOccupy++;
            addLog(QString("允许进行IO:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType));
        }
        else
        {
            this->waitingList.append(runningProc);
            refreshWaitingUI();
            addLog(QString("IO资源已满:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType).append("\n\t正进行").append(runningProc->eventType).append("IO的进程数:").append(QString::number(this->USBOccupy)));
        }
        break;
    case 2:runningProc->eventType = "PRINTER";
        addLog(QString("接受到IO申请:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType));
        if(PrinterOccupy < 1)
        {
            this->IOList.append(runningProc);
            refreshIOUI();
            this->PrinterOccupy++;
            addLog(QString("允许进行IO:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType));
        }
        else
        {
            this->waitingList.append(runningProc);
            refreshWaitingUI();
            addLog(QString("IO资源已满:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType).append("\n\t正进行").append(runningProc->eventType).append("IO的进程数:").append(QString::number(this->PrinterOccupy)));
        }
        break;
    case 3:runningProc->eventType = "DISK";
        addLog(QString("接受到IO申请:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType));
        if(DiskOccupy < 5)
        {
            this->IOList.append(runningProc);
            refreshIOUI();
            this->DiskOccupy++;
            addLog(QString("允许进行IO:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType));
        }
        else
        {
            this->waitingList.append(runningProc);
            refreshWaitingUI();
            addLog(QString("IO资源已满:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType).append("\n\t正进行").append(runningProc->eventType).append("IO的进程数:").append(QString::number(this->DiskOccupy)));
        }
        break;
    case 4:runningProc->eventType = "GRAPHICS";
        addLog(QString("接受到IO申请:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType));
        if(GraphicsOccupy < 3)
        {
            this->IOList.append(runningProc);
            refreshIOUI();
            this->GraphicsOccupy++;
            addLog(QString("允许进行IO:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType));
        }
        else
        {
            this->waitingList.append(runningProc);
            refreshWaitingUI();
            addLog(QString("IO资源已满:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:").append(runningProc->eventType).append("\n\t正进行").append(runningProc->eventType).append("IO的进程数:").append(QString::number(this->GraphicsOccupy)));
            addLog(QString("进入Wating队列:PID = ").append(QString::number(runningProc->getPid())).append("申请IO资源:"));
        }
        break;
    default:runningProc->eventType = "ERROR";
    }
    runningProc = nullptr;
    ui->label_rotation->setVisible(true);
    resetRunningUI();
}

void Simulator::refreshWaitingUI()
{
    QStringList waitingStringList;
    for (int i = 0;i < this->waitingList.length(); i++)
    {
        if(startMode == ROUND_ROBIN)
            waitingStringList<<"PID:"+QString::number(waitingList.at(i)->getPid())+"\n"+
                               "剩余时长:"+QString::number(waitingList.at(i)->getCalUseTime())+"            "+
                               "申请IO:"+waitingList.at(i)->eventType+"\n"+
                               "占用内存:"+QString::number(waitingList.at(i)->getNeededLength())+"\n";
        else
            waitingStringList<<"PID:"+QString::number(waitingList.at(i)->getPid())+"\n"+
                               "剩余时长:"+QString::number(waitingList.at(i)->getCalUseTime())+"            "+
                               "申请IO:"+waitingList.at(i)->eventType+"\n"+
                               "优先级:"+QString::number(waitingList.at(i)->getPriority())+"\n";
        "占用内存:"+QString::number(waitingList.at(i)->getNeededLength())+"\n";
    }
    QStringListModel* waitingStringListModel = new QStringListModel(waitingStringList);
    ui->listView_waiting->setModel(waitingStringListModel);
    ui->label_waitingLength->setText(QString::number(waitingList.length()));
}

void Simulator::refreshIOUI()
{
    if(!IOList.isEmpty())
        ui->label_IO_icon->setVisible(true);
    else ui->label_IO_icon->setVisible(false);
    QStringList IOStringList;
    for (int i = 0;i < this->IOList.length(); i++)
    {
        if(startMode == ROUND_ROBIN)
            IOStringList<<"PID:"+QString::number(IOList.at(i)->getPid())+"\n"+
                          "剩余时长:"+QString::number(IOList.at(i)->getCalUseTime())+"         "+
                          "申请IO:"+IOList.at(i)->eventType+"\n"+
                          "占用内存:"+QString::number(IOList.at(i)->getNeededLength())+"\n";
        else
            IOStringList<<"PID:"+QString::number(IOList.at(i)->getPid())+"\n"+
                          "剩余时长:"+QString::number(IOList.at(i)->getCalUseTime())+"         "+
                          "申请IO:"+IOList.at(i)->eventType+"\n"+
                          "优先级:"+QString::number(IOList.at(i)->getPriority())+"\n"
                                                                              "占用内存:"+QString::number(IOList.at(i)->getNeededLength())+"\n";
    }
    QStringListModel* IOStringListModel = new QStringListModel(IOStringList);
    ui->listView_IO->setModel(IOStringListModel);
    ui->label_IOLength->setText(QString::number(IOList.length()));
}


void Simulator::refreshMemoryUI()
{
    qDebug()<<"refreshMemoryUI!!!"<<endl;
    //从按钮组中删除所有按钮
    for (int i = 0;i < this->displayButtonList->length();i++)
    {
        delete this->displayButtonList->takeAt(i);
    }

    for (int i = 0;i < this->partitionTable.length();i++)
    {
        Partition* partitionToShow = partitionTable.at(i);

        QPushButton* displayButton = new QPushButton(this);
        displayButton->setEnabled(false);
        QString statusEmoji;
        if(partitionTable.at(i)->getStatus() == 0)
        {
            statusEmoji = "🔑";
            displayButton->setStyleSheet("background-color: rgb(166, 191, 75);color:rgb(0, 0, 0);font-size: 13px;font-family: \"Cascadia Code\", serif;");
        }
        else
        {
            statusEmoji = "🔒";
            displayButton->setStyleSheet("background-color: rgb(255, 102, 90);color:rgb(0, 0, 0);font-size: 13px;font-family: \"Cascadia Code\", serif;");

        }
        QString displayContent = QString::number(partitionToShow->getStart())+"\n"+
                statusEmoji+" LENGTH -> "+QString::number(partitionToShow->getLength())+"\n"+
                QString::number(partitionToShow->getLength()+partitionTable.at(i)->getStart());

        this->displayButtonList->append(displayButton);
        displayButton->setText(displayContent);

        double posY = ORIGIN_Y+TOTAL_HEIGHT*(1.0*partitionToShow->getStart()/memorySize);
        double height = TOTAL_HEIGHT*(1.0*partitionToShow->getLength()/memorySize);
        displayButton->setGeometry(ORIGIN_X,static_cast<int>(posY),
                                   STANDARD_W,static_cast<int>(height));
        displayButton->show();
        testInt+=5;
    }
}

/*
 * 此前的版本
 */
//void Simulator::refreshMemoryUI()
//{
//    QStringList partititonStringList;
//    for (int i = 0;i < this->partitionTable.length(); i++)
//    {
//        QString statusEmoji;
//        if(partitionTable.at(i)->getStatus() == 0)
//            statusEmoji = "✔";
//        else
//            statusEmoji = "🔒";

//        if(i == 0)
//        {
//            partititonStringList<<QString::number(partitionTable.at(i)->getStart())+"\n\n"+
//                                  "LENGTH -> "+QString::number(partitionTable.at(i)->getLength())+"\n"+
//                                  "STATUS -> "+statusEmoji+"\n\n"+
//                                  QString::number(partitionTable.at(i)->getLength()+partitionTable.at(i)->getStart());
//       }
//        else {
//            partititonStringList<<"\nLENGTH -> "+QString::number(partitionTable.at(i)->getLength())+"\n"
//                                  "STATUS -> "+statusEmoji+"\n\n"+
//                                  QString::number(partitionTable.at(i)->getLength()+partitionTable.at(i)->getStart());
//        }

//    }
//    QStringListModel* partititonStringListModel = new QStringListModel(partititonStringList);
//    ui->listView_memory->setModel(partititonStringListModel);
//}

void Simulator::loadIOProc(QString IOType)
{
    foreach(PCB* pcb,waitingList)
    {
        if(IOType == pcb->eventType)
        {
            IOList.append(pcb);
            waitingList.removeOne(pcb);
            refreshWaitingUI();
            refreshIOUI();
            addLog("新批准IO:PID = "+QString::number(pcb->getPid())+"\n\t"+"申请IO:"+pcb->eventType);
            if(IOType == "USB")
                this->USBOccupy++;
            else if(IOType == "PRINTER")
                this->PrinterOccupy++;
            else if(IOType == "DISK")
                this->DiskOccupy++;
            else this->GraphicsOccupy++;

            return;
        }
    }
}

void Simulator::IOAction()
{
    QString releaseType;
    PCB* pcb = IOList.takeFirst();

    //IO完毕，释放内存资源
    this->releasePartition(pcb->getStartingPos());
    addLog(QString("PID = ").append(QString::number(pcb->getPid())).append(" -> IO完毕 释放内存").append("起址:").append(QString::number(pcb->getStartingPos())));

    this->backupProcList.append(pcb);
    this->loadProc();


    refreshReadyUI();
    refreshIOUI();

    addLog(QString("PID = ").append(QString::number(pcb->getPid())).append(" -> IO完毕").append("申请IO资源:").append(pcb->eventType));
    releaseType = pcb->eventType;


    /*
     * 释放对应的IO资源
     */
    if(pcb->eventType == "USB")
    {
        this->USBOccupy--;
        this->addLog(QString("IO资源已释放 -> USB").append("此刻的占用数: ").append(QString::number(this->USBOccupy)));
    }
    else if(pcb->eventType == "PRINTER")
    {
        this->PrinterOccupy--;
        this->addLog(QString("IO资源已释放 -> USB").append("此刻的占用数: ").append(QString::number(this->PrinterOccupy)));
    }
    else if(pcb->eventType == "DISK")
    {
        this->DiskOccupy--;
        this->addLog(QString("IO资源已释放 -> USB").append("此刻的占用数: ").append(QString::number(this->DiskOccupy)));
    }
    else
    {
        this->GraphicsOccupy--;
        this->addLog(QString("IO资源已释放 -> USB").append("此刻的占用数: ").append(QString::number(this->GraphicsOccupy)));
    }
    pcb->eventType = "N/A";

    qDebug()<<"releaseType == == == "<<releaseType<<endl;
    loadIOProc(releaseType);
    return;
}

void Simulator::automaticRun()
{
    switch (this->startMode)
    {
    case 1195:preemptiveProrityAction();break;
    case 1196:nonPriorityAction();break;
    case 1197:roundRobinAction();break;
    default:qDebug()<<"选择执行Action时出错！"<<endl;
    }
    IOCount++;

    if(IOCount == autoIOGap)
    {
        if(!IOList.isEmpty())
            this->IOAction();
        IOCount = 0;
    }
}

int Simulator::firstFitAction(int neededLength)
{
    //遍历一次分区表，找到第一个放得下的位置
    for (int i = 0;i < this->partitionTable.length();i++)
    {
        if(partitionTable.at(i)->getLength() == neededLength && partitionTable.at(i)->getStatus() == 0)//刚刚好放得下
        {
            partitionTable.at(i)->setStatus(1);
            return partitionTable.at(i)->getStart();
        }
        if(partitionTable.at(i)->getLength() > neededLength && partitionTable.at(i)->getStatus() == 0)
        {
            Partition* suitPartition = partitionTable.at(i);
            Partition* newPartition = new Partition(suitPartition->getStart()+neededLength,suitPartition->getLength()-neededLength,0);
            suitPartition->setLength(neededLength);
            suitPartition->setStatus(1);
            this->partitionTable.insert(i+1,newPartition);
            return suitPartition->getStart();
        }
    }

    //到这里还没有return，说明没有找到一个合适的分区容纳这个进程
    //求取总空闲空间，看看空闲的总内存是否能够满足
    if(this->getFreeMemorySize() >= neededLength)//能
    {
        //收缩内存
        shrinkAction(neededLength);
        //重新递归调用
        firstFitAction(neededLength);
    }
    else//不能
        return -1;
}

void Simulator::releasePartition(int startingPos)
{
    for (int i = 0;i < this->partitionTable.length();i++)
    {
        if(partitionTable.at(i)->getStart() == startingPos)
        {
            Partition* partitionToRelease = partitionTable.at(i);
            partitionToRelease->setStatus(0);
            //看看后一块空间是不是空的
            if(i + 1 < partitionTable.length() && partitionTable.at(i+1)->getStatus() == 0)//空的，吸纳进来
            {
                partitionToRelease->setLength(partitionToRelease->getLength()+partitionTable.at(i+1)->getLength());
                partitionTable.removeAt(i+1);
            }
            //看看前一块空间是不是空的
            if(i - 1 >= 0 && partitionTable.at(i-1)->getStatus() == 0)//空的，把这块放到前面去
            {
                partitionTable.at(i-1)->setLength(partitionTable.at(i-1)->getLength()+partitionToRelease->getLength());
                partitionTable.removeAt(i);
            }
            this->refreshMemoryUI();
            return;
        }
    }
}

int Simulator::getFreeMemorySize()
{
    int freeMemorySize = 0;
    foreach(Partition* partition,this->partitionTable)
    {
        if(partition->getStatus() == 0)
        {
            freeMemorySize += partition->getLength();
        }
    }
    return freeMemorySize;
}



void Simulator::shrinkAction(int neededLength)
{
    addLog("shrinkAction is CALLED!");
    int testLength = 0;
    while(true)
    {
        qDebug()<<"while迭代";
        for (int i = 0;i < this->partitionTable.length();i++)
        {
            qDebug()<<"进for"<<endl;
            qDebug()<<"this->partitionTable.length()"<<this->partitionTable.length()<<endl;
            if(i+1 < partitionTable.length() && partitionTable.at(i)->getStatus() == 0)
            {
                //SWAP

                qDebug()<<partitionTable.at(i)->getStart()<<"  "<<partitionTable.at(i)->getStatus()<<"   "<<partitionTable.at(i)->getLength();
                qDebug()<<partitionTable.at(i+1)->getStart()<<"  "<<partitionTable.at(i+1)->getStatus()<<"   "<<partitionTable.at(i+1)->getLength();
                int start = partitionTable.at(i)->getStart();
                int status = partitionTable.at(i)->getStatus();
                int length = partitionTable.at(i)->getLength();

                partitionTable.at(i)->setStart(partitionTable.at(i+1)->getStart());
                partitionTable.at(i)->setStatus(partitionTable.at(i+1)->getStatus());
                partitionTable.at(i)->setLength(partitionTable.at(i+1)->getLength());

                partitionTable.at(i+1)->setStart(start);
                partitionTable.at(i+1)->setStatus(status);
                partitionTable.at(i+1)->setLength(length);

                qDebug()<<partitionTable.at(i)->getStart()<<"  "<<partitionTable.at(i)->getStatus()<<"   "<<partitionTable.at(i)->getLength();
                qDebug()<<partitionTable.at(i+1)->getStart()<<"  "<<partitionTable.at(i+1)->getStatus()<<"   "<<partitionTable.at(i+1)->getLength();
                qDebug()<<"交换完成了！！！！！！！！！！"<<endl;
                this->refreshMemoryUI();
                //交换完成之后看看能不能与后一块合并
                if(i+1+1 < partitionTable.length() && partitionTable.at(i+1+1)->getStatus() == 0)
                {
                    qDebug()<<"开始合并"<<endl;
                    partitionTable.at(i+1)->setLength(partitionTable.at(i+1)->getLength() + partitionTable.at(i+1+1)->getLength());
                    testLength = partitionTable.at(i+1)->getLength();
                    this->partitionTable.removeAt(i+1+1);
                    qDebug()<<"合并完成"<<endl;
                }
                qDebug()<<"break前"<<endl;
                break;
            }
        }
        qDebug()<<"break后"<<endl;

        //看看收缩出来的空间是否满足了需求
        if(testLength >= neededLength)
        {
            qDebug()<<"判断"<<endl;
//            this->addLog("收缩了内存空间，收缩空间起址:"+QString::number(shrinkedPartition->getStart())+"\n\t"+
//                         "收缩出的总长度:"+QString::number(shrinkedPartition->getLength()));
            return;
        }
        qDebug()<<"准备再循环"<<endl;
    }
}

void Simulator::IOAll()
{
    while(!IOList.isEmpty())
    {
        IOAction();
    }
}

//从创建进程窗口接受新的PCB
void Simulator::getNewPCB(PCB *pcb)
{
    if(readyList.length() < MAX_PROGRAM_AMOUNT)//小于道数就直接载入
    {
        this->readyList.append(pcb);
        addLog(QString("添加了新进程：PID = ").append(QString::number(pcb->getPid())));
        addLog(QString("装载进程：PID = ").append(QString::number(pcb->getPid())).append("至Ready队列"));
        refreshReadyUI();
    }
    else
    {
        this->backupProcList.append(pcb);
        addLog(QString("添加了新进程：PID = ").append(QString::number(pcb->getPid())));
        refreshBackupUI();
    }
}

//弹出进程创建窗口
void Simulator::on_pushButton_custom_clicked()
{
    ProcessCreate* processCreatePanel = new ProcessCreate();
    connect(processCreatePanel,SIGNAL(transmitPCB(PCB*)),this,SLOT(getNewPCB(PCB*)));
    processCreatePanel->show();
}

void Simulator::on_pushButton_random_clicked()
{
    qsrand(QTime::currentTime().msec()*qrand()*qrand()*qrand()*qrand()*qrand()*qrand());
    int randPID;
    int randTime;
    int randPriority;
    int randNeededLength;

    do {randTime = qrand()%(maxTime+1);} while (randTime <= 0);
    do {randPID = qrand()%(maxPID+1);} while (randPID <= 0);
    do {randPriority = qrand()%(maxPriority+1);} while (randTime <= 0);
    do {randNeededLength = qrand()%(maxNeededLength+1);} while (randNeededLength < minNeededLength);

    PCB* newPCB = new PCB(randPID,randTime,randPriority,randNeededLength);
    addLog(QString("生成了新进程：PID = ").append(QString::number(newPCB->getPid())));
    this->backupProcList.append(newPCB);
    this->loadProc();
    this->refreshBackupUI();

    //    if(readyList.length() < MAX_PROGRAM_AMOUNT)
    //    {
    //        addLog(QString("装载进程：PID = ").append(QString::number(newPCB->getPid())).append("至Ready队列"));
    //        this->readyList.append(newPCB);
    //        refreshReadyUI();
    //    }
    //    else
    //    {
    //        this->backupProcList.append(newPCB);
    //        refreshBackupUI();
    //    }
}

void Simulator::addLog(QString content)
{
    content = content.toUtf8();
    QDateTime current_time = QDateTime::currentDateTime();
    QString currentTime = current_time.toString("yyyy-MM-dd hh:mm:ss");
    QString log = "["+currentTime+"]: "+content;
    log = log.toUtf8();
    ui->textBrowser->append(log);
    qDebug()<<log<<endl;
}

PCB *Simulator::getTopPriority()
{
    int tempPri = readyList.first()->getPriority();
    PCB* tempPCB = readyList.first();
    foreach(PCB* pcb,this->readyList)
    {
        if(pcb->getPriority()<tempPri)
        {
            tempPri = pcb->getPriority();
            tempPCB = pcb;
        }
    }
    return tempPCB;
}

void Simulator::nonPriorityAction()
{
    if(this->readyList.isEmpty()&&runningProc == nullptr)
        return;
    this->loadProc();
    //先看看有没有在运行的进程
    if(this->runningProc == nullptr)//没有
    {
        this->runningProc = getTopPriority();
        ui->label_rotation->setVisible(false);
        readyList.removeOne(this->runningProc);
        this->runningProc->timeDecline();
        addLog("PID:"+QString::number(runningProc->getPid())+" -> 运行了一个时间单位"+"\n\t\t"
                                                                             "剩余运行时间:"+QString::number(runningProc->getCalUseTime()));
        if(runningProc->isTerminated())
        {
            addLog("PID:"+QString::number(runningProc->getPid())+" -> 现已终结"+"\n\t\t"
                                                                            "总耗时:"+QString::number(runningProc->getNeededTime()));
            this->terminatedList.append(runningProc);
            //释放内存
            this->releasePartition(runningProc->getStartingPos());
            addLog(QString("PID = ").append(QString::number(runningProc->getPid())).append(" -> 释放内存").append("起址:").append(QString::number(runningProc->getStartingPos())));
            refreshRunningUI();
            refreshTerminatedUI();
            this->runningProc = nullptr;
            if(this->readyList.isEmpty())
            {
                resetRunningUI();
                ui->label_rotation->setVisible(false);
                addLog("全部进程已执行完毕！");
            }
        }
        else
            refreshRunningUI();

        refreshReadyUI();
    }
    else//有
    {
        runningProc->timeDecline();
        addLog("PID:"+QString::number(runningProc->getPid())+" -> 运行了一个时间单位"+"\n\t\t"
                                                                             "剩余运行时间:"+QString::number(runningProc->getCalUseTime()));
        refreshRunningUI();
        if(runningProc->isTerminated())
        {
            addLog("PID:"+QString::number(runningProc->getPid())+" -> 现已终结"+"\n\t\t"
                                                                            "总耗时:"+QString::number(runningProc->getNeededTime()));
            ui->label_rotation->setVisible(true);
            this->terminatedList.append(runningProc);
            //释放内存
            this->releasePartition(runningProc->getStartingPos());
            addLog(QString("PID = ").append(QString::number(runningProc->getPid())).append(" -> 释放内存").append("起址:").append(QString::number(runningProc->getStartingPos())));
            refreshTerminatedUI();
            this->runningProc = nullptr;
            resetRunningUI();
            if(this->readyList.isEmpty())
            {

                if(IOList.isEmpty())
                {
                    addLog("全部进程已执行完毕！");
                    ui->label_rotation->setVisible(false);
                    timer->stop();
                }
                else
                    IOAll();
            }
        }
    }
    //老化
    foreach(PCB* pcb,readyList)
    {
        pcb->waitingTimeInc();
        if(pcb->getWaitingTime() >= agingTime)
        {
            this->addLog("PID:"+QString::number(pcb->getPid())+" -> 等待时间超过阈值，老化"+"\n\t\t"+
                         "目前优先级:"+QString::number(pcb->getPriority()));
            pcb->aging();
            pcb->resetWaitingTime();
        }
    }
    ramdomlyEvent();
}

void Simulator::preemptiveProrityAction()
{
    if(this->readyList.isEmpty() &&runningProc == nullptr)
        return;
    this->loadProc();
    //先看看有没有在运行的进程
    if(this->runningProc == nullptr)//没有
    {
        this->runningProc = getTopPriority();
        readyList.removeOne(this->runningProc);
        ui->label_rotation->setVisible(false);
        this->runningProc->timeDecline();
        addLog("PID:"+QString::number(runningProc->getPid())+" -> 运行了一个时间单位"+"\n\t\t"
                                                                             "剩余运行时间:"+QString::number(runningProc->getCalUseTime()));
        if(runningProc->isTerminated())
        {
            addLog("PID:"+QString::number(runningProc->getPid())+" -> 现已终结"+"\n\t\t"
                                                                            "总耗时:"+QString::number(runningProc->getNeededTime()));
            this->terminatedList.append(runningProc);
            //释放内存
            this->releasePartition(runningProc->getStartingPos());
            addLog(QString("PID = ").append(QString::number(runningProc->getPid())).append(" -> 释放内存").append("起址:").append(QString::number(runningProc->getStartingPos())));
            refreshTerminatedUI();
            this->runningProc = nullptr;
            resetRunningUI();
            if(this->readyList.isEmpty())
            {

                if(IOList.isEmpty())
                {
                    timer->stop();
                    ui->label_rotation->setVisible(false);
                    addLog("全部进程已执行完毕！");
                }
                else
                    IOAll();
            }
        }
        else
            refreshRunningUI();

        refreshReadyUI();
    }
    else
    {
        if(!this->readyList.isEmpty())
        {
            PCB* topPriorityReady = getTopPriority();
            if(topPriorityReady->getPriority() < this->runningProc->getPriority())//如果找到了新的高优先
            {
                //把旧的赶下来
                addLog("PID:"+QString::number(runningProc->getPid()) + " 的优先级低于 PID:" + QString::number(topPriorityReady->getPid())+"\n\t\t"+"发生抢占!");
                this->readyList.append(this->runningProc);
                this->runningProc = nullptr;
                ui->label_rotation->setVisible(true);
                this->runningProc = topPriorityReady;
                this->readyList.removeOne(this->runningProc);
                ui->label_rotation->setVisible(false);
                refreshReadyUI();
                refreshRunningUI();
            }
        }

        runningProc->timeDecline();
        addLog("PID:"+QString::number(runningProc->getPid())+" -> 运行了一个时间单位"+"\n\t\t"
                                                                             "剩余运行时间:"+QString::number(runningProc->getCalUseTime()));
        refreshRunningUI();
        if(runningProc->isTerminated())
        {
            addLog("PID:"+QString::number(runningProc->getPid())+" -> 现已终结"+"\n\t\t"
                                                                            "总耗时:"+QString::number(runningProc->getNeededTime()));
            this->terminatedList.append(runningProc);
            //释放内存
            this->releasePartition(runningProc->getStartingPos());
            addLog(QString("PID = ").append(QString::number(runningProc->getPid())).append(" -> 释放内存").append("起址:").append(QString::number(runningProc->getStartingPos())));
            refreshTerminatedUI();
            this->runningProc = nullptr;
            ui->label_rotation->setVisible(true);
            resetRunningUI();
            if(this->readyList.isEmpty())
            {

                if(IOList.isEmpty())
                {
                    addLog("全部进程已执行完毕！");
                    ui->label_rotation->setVisible(false);
                    timer->stop();
                }
                else
                    IOAll();
            }
        }
    }
    //老化
    foreach(PCB* pcb,readyList)
    {
        pcb->waitingTimeInc();
        if(pcb->getWaitingTime() >= agingTime)
        {
            this->addLog("PID:"+QString::number(pcb->getPid())+" -> 等待时间超过阈值，老化"+"\n\t\t"+
                         "目前优先级:"+QString::number(pcb->getPriority()));
            pcb->aging();
            pcb->resetWaitingTime();
        }
    }
    ramdomlyEvent();
}

void Simulator::roundRobinAction()
{
    this->loadProc();
    if(this->runningProc == nullptr)//如果目前没有正在运行的进程
    {
        if(readyList.isEmpty())
            return;
        this->runningProc = this->readyList.takeFirst();
        ui->label_rotation->setVisible(false);
        this->runningProc->timeDecline();
        this->runningProc->usedTimeSliceInc();
        addLog("PID:"+QString::number(runningProc->getPid())+" -> 运行了一个时间单位"+"\n\t\t"
                                                                             "剩余运行时间:"+QString::number(runningProc->getCalUseTime())+"\n\t\t"
               + "剩余时间片:"+QString::number(this->TIME_SLICE - runningProc->getUsedTimeSlice()));
        if(runningProc->isTerminated())
        {
            addLog("PID:"+QString::number(runningProc->getPid())+" -> 现已终结"+"\n\t\t"
                                                                            "总耗时:"+QString::number(runningProc->getNeededTime()));
            this->terminatedList.append(runningProc);
            //释放内存
            this->releasePartition(runningProc->getStartingPos());
            addLog(QString("PID = ").append(QString::number(runningProc->getPid())).append(" -> 释放内存").append("起址:").append(QString::number(runningProc->getStartingPos())));
            refreshTerminatedUI();
            this->runningProc = nullptr;
            resetRunningUI();
            if(this->readyList.isEmpty())
            {
                addLog("全部进程已执行完毕！");
                if(IOList.isEmpty())
                    timer->stop();
                else
                    IOAll();
            }
        }
        else
            refreshRunningUI();

        refreshReadyUI();
    }
    else
    {
        this->runningProc->timeDecline();
        this->runningProc->usedTimeSliceInc();
        refreshRunningUI();
        addLog("PID:"+QString::number(runningProc->getPid())+" -> 运行了一个时间单位"+"\n\t\t"
                                                                             "剩余运行时间:"+QString::number(runningProc->getCalUseTime())+"\n\t\t"
               + "剩余时间片:"+QString::number(this->TIME_SLICE - runningProc->getUsedTimeSlice()));
        if(runningProc->isTerminated())
        {
            addLog("PID:"+QString::number(runningProc->getPid())+" -> 现已终结"+"\n\t\t"
                                                                            "总耗时:"+QString::number(runningProc->getNeededTime()));
            this->terminatedList.append(runningProc);
            //释放内存
            this->releasePartition(runningProc->getStartingPos());
            addLog(QString("PID = ").append(QString::number(runningProc->getPid())).append(" -> 释放内存").append("起址:").append(QString::number(runningProc->getStartingPos())));
            refreshTerminatedUI();
            this->runningProc = nullptr;
            resetRunningUI();
            if(this->readyList.isEmpty())
            {
                addLog("全部进程已执行完毕！");
                if(IOList.isEmpty())
                    timer->stop();
                else
                    IOAll();
            }
        }
        else{
            if(runningProc->getUsedTimeSlice() >= this->TIME_SLICE)//用完了所有的时间片
            {
                addLog("PID:"+QString::number(runningProc->getPid())+" -> 用完了所有的时间片"+"\n\t\t"
                                                                                     "剩余运行时间:"+QString::number(runningProc->getCalUseTime())+"\n\t\t"
                       +"发生轮转");
                this->runningProc->resetUsedTimeSlice();
                if(!readyList.isEmpty())
                {
                    //排到队尾
                    this->readyList.append(this->runningProc);
                    this->runningProc = nullptr;
                    ui->label_rotation->setVisible(true);
                }
                else//如果ready队列没有了，不再限制正在运行的时间片
                {
                    addLog("PID:"+QString::number(runningProc->getPid())+" -> 不限制此次时间片"+"\n\t\t"
                                                                                        "剩余运行时间:"+QString::number(runningProc->getCalUseTime())+"\n\t\t"
                           + "剩余时间片:"+QString::number(this->TIME_SLICE - runningProc->getUsedTimeSlice())
                           +"\n\t\t"
                           +"发生轮转");
                    this->runningProc->resetUsedTimeSlice();
                }
            }
        }
        refreshReadyUI();
    }
    ramdomlyEvent();
}

void Simulator::on_pushButton_manual_clicked()
{
    switch (this->startMode)
    {
    case 1195:preemptiveProrityAction();break;
    case 1196:nonPriorityAction();break;
    case 1197:roundRobinAction();break;
    default:qDebug()<<"选择执行Action时出错！"<<endl;
    }
}


void Simulator::on_pushButton_IO_clicked()
{
    if(ui->listView_IO->selectionModel() != nullptr)
    {
        QStringListModel* model = qobject_cast<QStringListModel *>(ui->listView_IO->model());
        QModelIndexList modelIndexList = ui->listView_IO->selectionModel()->selectedIndexes();
        QString releaseType;
        foreach(QModelIndex modelIndex, modelIndexList)
        {
            QString pid =  model->data(modelIndex).toString().section("\n",0,0).section(":",1,1);
            foreach(PCB* pcb,IOList)
            {
                if(pcb->getPid() == pid.toInt())
                {
                    this->readyList.append(pcb);
                    IOList.removeOne(pcb);

                    //IO完毕，释放内存资源
                    this->releasePartition(pcb->getStartingPos());
                    addLog(QString("PID = ").append(QString::number(pcb->getPid())).append(" -> IO完毕 释放内存").append("起址:").append(QString::number(pcb->getStartingPos())));

                    refreshReadyUI();
                    refreshIOUI();

                    addLog(QString("PID = ").append(QString::number(pcb->getPid())).append(" -> IO完毕 ").append("申请IO资源:").append(pcb->eventType));
                    releaseType = pcb->eventType;

                    if(pcb->eventType == "USB")
                    {
                        this->USBOccupy--;
                        this->addLog(QString("IO资源已释放 -> USB").append("此刻的占用数: ").append(QString::number(this->USBOccupy)));
                    }
                    else if(pcb->eventType == "PRINTER")
                    {
                        this->PrinterOccupy--;
                        this->addLog(QString("IO资源已释放 -> USB").append("此刻的占用数: ").append(QString::number(this->PrinterOccupy)));
                    }
                    else if(pcb->eventType == "DISK")
                    {
                        this->DiskOccupy--;
                        this->addLog(QString("IO资源已释放 -> USB").append("此刻的占用数: ").append(QString::number(this->DiskOccupy)));
                    }
                    else
                    {
                        this->GraphicsOccupy--;
                        this->addLog(QString("IO资源已释放 -> USB").append("此刻的占用数: ").append(QString::number(this->GraphicsOccupy)));
                    }
                    pcb->eventType = "N/A";
                }
            }
        }
        loadIOProc(releaseType);
        return;
    }
}

void Simulator::on_pushButton_suspend_clicked()
{
    if(readyList.isEmpty()&&runningProc == nullptr)
        return;
    //挂起Ready队列的进程
    QStringListModel* model = qobject_cast<QStringListModel *>(ui->listView_ready->model());
    QModelIndexList modelIndexList = ui->listView_ready->selectionModel()->selectedIndexes();
    foreach(QModelIndex modelIndex, modelIndexList)
    {
        QString pid =  model->data(modelIndex).toString().section("\n",0,0).section(":",1,1);
        foreach(PCB* pcb,readyList)
        {
            if(pcb->getPid() == pid.toInt())
            {
                this->suspendedList.append(pcb);
                readyList.removeOne(pcb);
                refreshReadyUI();
                break;
            }
        }
    }

    if(runningProc!=nullptr)
    {
        //挂起运行中的进程
        model = qobject_cast<QStringListModel *>(ui->listView_running->model());
        modelIndexList = ui->listView_running->selectionModel()->selectedIndexes();
        foreach(QModelIndex modelIndex, modelIndexList)
        {
            QString pid =  model->data(modelIndex).toString().section("\n",0,0).section(":",1,1);
            if(runningProc->getPid() == pid.toInt())
            {
                this->suspendedList.append(this->runningProc);
                runningProc = nullptr;
                resetRunningUI();
                break;
            }
        }
    }

    if(!suspendedList.isEmpty())
        refreshSuspendedUI();
}

void Simulator::on_pushButton_auto_clicked()
{
    if(readyList.isEmpty()&&runningProc == nullptr)
        return;
    timer->start(timeScale);
}


void Simulator::on_pushButton_IO_All_clicked()
{
    while(!IOList.isEmpty())
    {
        IOAction();
    }
}

void Simulator::on_pushButton_suspend_off_clicked()
{
    //解除挂起
    //先检测是否选中的是挂起列表里面的元素
    qDebug()<<(ui->listView_suspended->selectionModel() == nullptr);
    if(ui->listView_suspended->selectionModel() != nullptr)
    {
        QStringListModel* model = qobject_cast<QStringListModel *>(ui->listView_suspended->model());
        QModelIndexList modelIndexList = ui->listView_suspended->selectionModel()->selectedIndexes();
        foreach(QModelIndex modelIndex, modelIndexList)
        {
            QString pid =  model->data(modelIndex).toString().section("\n",0,0).section(":",1,1);
            foreach(PCB* pcb,suspendedList)
            {
                if(pcb->getPid() == pid.toInt())
                {
                    this->readyList.append(pcb);
                    suspendedList.removeOne(pcb);
                    refreshReadyUI();
                    refreshSuspendedUI();
                    break;
                }
            }
        }
        ui->listView_suspended->selectionModel()->clear();
        return;
    }

}

void Simulator::on_pushButton_auto_stop_clicked()
{
    timer->stop();
}

void Simulator::on_spinBox_autoIOGap_valueChanged(int arg1)
{
    timer->stop();
    this->autoIOGap = arg1;
}

void Simulator::on_spinBox_IORate_valueChanged(int arg1)
{
    timer->stop();
    this->randomlyEventRate = arg1;
}

void Simulator::on_spinBox_timeSlice_valueChanged(int arg1)
{
    timer->stop();
    this->TIME_SLICE = arg1;
}

void Simulator::on_spinBox_maxProcAmount_valueChanged(const QString &arg1)
{
    timer->stop();
    this->MAX_PROGRAM_AMOUNT = arg1.toInt();
}


void Simulator::on_lineEdit_timeScale_textChanged(const QString &arg1)
{
    this->timer->stop();
    this->timeScale = arg1.toInt();
    this->timer->start(this->timeScale);
}
