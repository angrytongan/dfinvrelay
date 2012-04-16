#ifndef SMA_H
#define SMA_H

bool initialiseSMAConnection(void);
bool logonSMAInverter(void);
unsigned long getInstantACPower(void);
unsigned long getTotalPowerGeneration(void);
unsigned long getLastDateTime(void);

#endif
