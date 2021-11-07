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
    int calUseTime;
    int waitingTime;
    int usedTimeSlice;


    PCB();

public:
    PCB(int pid, int neededTime, int priority);

    QString eventType;

    void setPriority(int priority);
    void setPid(int pid);
    void setStatus(int status);
    void setNeededTime(int neededTime);

    int getPriority();
    int getPid();
    int getNeededTime();
    int getStatus();
    int getCalUseTime();
    int getWaitingTime();
    int getUsedTimeSlice();

    void timeDecline();
    bool isTerminated();
    void aging();
    void waitingTimeInc();
    void resetWaitingTime();
    void resetUsedTimeSlice();
    void usedTimeSliceInc();
};

#endif // PCB_H
