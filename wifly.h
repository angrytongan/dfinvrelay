#ifndef WIFLY_H
#define WIFLY_H

#include <Time.h>
#include "relay.h"

#ifndef DESKTOP
SoftwareSerial wifly(RX_WIFLY, TX_WIFLY);
#endif

void wifly_init(void);
void wifly_upload_stats(unsigned long total_kwh, unsigned long spot_ac, time_t now, float temp);
void wifly_sleep(unsigned char minutes);

#endif
