#ifndef BT_H
#define BT_H

bool bt_init(void);
bool bt_get_status(unsigned long *total_kwh, unsigned long *spot_ac, unsigned long *now);

#endif
