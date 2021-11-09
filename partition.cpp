#include "partition.h"

Partition::Partition(int start, int length, int status) {
    this->start = start;
    this->length = length;
    this->status = status;
}

int Partition::getStart() const {
    return start;
}

void Partition::setStart(int start) {
    Partition::start = start;
}

int Partition::getLength() const {
    return length;
}

void Partition::setLength(int length) {
    Partition::length = length;
}

int Partition::getStatus() const {
    return status;
}

Partition::Partition() {}

void Partition::setStatus(int status) {
    Partition::status = status;
}
