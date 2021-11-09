#include "pcb.h"

PCB::PCB() {}

PCB::PCB(int pid, int neededTime, int priority,int neededLength)
{
    this->pid = pid;
    this->neededTime = neededTime;
    this->calUseTime = this->neededTime;
    this->priority = priority;
    this->status = 0;
    this->waitingTime = 0;
    this->usedTimeSlice = 0;
    this->eventType = "N/A";
    this->neededLength = neededLength;
    this->startingPos = 0;
    /* 0 -> Ready
     * 1 -> Running
     * 2 -> Waiting
     * 3 -> Suspended
     * 4 -> Terminated
     */
}


void PCB::setPriority(int priority)
{
    this->priority = priority;
}

void PCB::setPid(int pid)
{
    this->pid = pid;
}


void PCB::setStatus(int status)
{
    this->status = status;
}

void PCB::setNeededTime(int neededTime)
{
    this->neededTime = neededTime;
}

void PCB::setNeededLength(int neededLength)
{
    this->neededLength = neededLength;
}

void PCB::setStartingPos(int startingPos)
{
    this->startingPos = startingPos;
}

int PCB::getPid()
{
    return this->pid;
}

int PCB::getNeededTime()
{
    return this->neededTime;
}

int PCB::getPriority()
{
    return this->priority;
}

int PCB::getStatus()
{
    return this->status;
}

int PCB::getCalUseTime()
{
    return this->calUseTime;
}

int PCB::getWaitingTime()
{
    return this->waitingTime;
}

int PCB::getUsedTimeSlice()
{
    return this->usedTimeSlice;
}

int PCB::getNeededLength()
{
    return this->neededLength;
}

int PCB::getStartingPos()
{
    return this->startingPos;
}

void PCB::timeDecline()
{
    this->calUseTime = this->calUseTime - 1;
}

bool PCB::isTerminated()
{
    if(this->calUseTime == 0)
        return true;
    return false;
}

void PCB::aging()
{
    this->priority = this->priority - 1;
}

void PCB::waitingTimeInc()
{
    this->waitingTime = this->waitingTime+1;
}

void PCB::resetWaitingTime()
{
    this->waitingTime = 0;
}

void PCB::resetUsedTimeSlice()
{
    this->usedTimeSlice = 0;
}

void PCB::usedTimeSliceInc()
{
    this->usedTimeSlice = this->usedTimeSlice + 1;
}
