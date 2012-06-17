#ifndef RELAY_H
#define RELAY_H

#ifdef DESKTOP
#include "desktop.h"
#else

#include <avr/pgmspace.h>
#include <SoftwareSerial.h>
#include <Time.h>

/*
 * Pins.
 */
#define RX_BTBEE    0   /* Normal serial port */
#define TX_BTBEE    1   /* Normal serial port */
#define WIFLY_WAKE  2   /* Wifly interrupt; wakes MCU */
#define CD_BTBEE    3   /* connect to pio1 to detect connection status */
#define POWER_BTBEE 5   /* turn on / off BT (Stalker v2.0) */
#define RX_WIFLY    11  /* Stalker tx to wifly */
#define TX_WIFLY    12  /* Stalker rx from wifly */
#define DISCONNECT_BTBEE 13 /* connect to pio0; high will disconnect */

#define HOST_LED    8

#endif

#include "bt.h"
#include "wifly.h"

#endif
