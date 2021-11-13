#ifndef PARTITION_H
#define PARTITION_H

#include "pcb.h"

class Partition {
private:
    int start;
    int length;


    /*
     * 0 -> 未分
     * 1 -> 已分
     */
    int status;
public:
    PCB* associatedPCB;

    int getStart() const;

    void setStart(int start);

    int getLength() const;

    void setLength(int length);

    int getStatus() const;

    void setStatus(int status);


public:
    Partition();
    Partition(int start, int length, int status);
};

#endif // PARTITION_H
