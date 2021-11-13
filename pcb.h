#ifndef PCB_H
#define PCB_H

#include <QString>

class PCB
{
private:
    int pid;
    int neededTime;
    int priority;

    int status;
    /*
     * 0 -> backup
     * 1 -> ready
     * 2 -> running
     * 3 -> waiting
     * 4 -> IO
     * 5 -> suspended
     * 6 -> terminated
     */

    int calUseTime;
    int waitingTime;
    int usedTimeSlice;

    //内存分配相关
    int neededLength;
    int startingPos;

    PCB();

public:
    PCB(int pid, int neededTime, int priority,int neededLength);

    QString eventType;

    void setPriority(int priority);
    void setPid(int pid);
    void setStatus(int status);
    void setNeededTime(int neededTime);
    void setNeededLength(int neededLength);
    void setStartingPos(int startingPos);

    int getPriority();
    int getPid();
    int getNeededTime();
    int getStatus();
    int getCalUseTime();
    int getWaitingTime();
    int getUsedTimeSlice();
    int getNeededLength();
    int getStartingPos();

    void timeDecline();
    bool isTerminated();
    void aging();
    void waitingTimeInc();
    void resetWaitingTime();
    void resetUsedTimeSlice();
    void usedTimeSliceInc();
};

#endif // PCB_H
