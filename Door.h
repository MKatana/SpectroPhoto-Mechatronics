#ifndef DOOR_H
#define DOOR_H

extern bool interrupted;

void doorInterruptInit();
bool processDoorInterrupt(bool allowReinitialize = true);

#endif
