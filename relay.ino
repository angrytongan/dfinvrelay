#include "relay.h"
#ifndef DESKTOP
#include <avr/sleep.h>
#include <SoftwareSerial.h>
#include <Time.h>
#include <Wire.h>
#include "util.h"
#endif

/*
 * Faster baud rates seem to be dropping characters, need
 * to do more definitive testing at faster rates. We aren't
 * terribly concerned about speed, so it's fine to use
 * slightly slower speeds.
 */
#define WIFLY_BAUD_RATE    19200
#define BTBEE_BAUD_RATE    9600

#define MAX_RETRIES 3

unsigned long last_total_kwh;
unsigned long last_spot_ac;
unsigned long last_now;
int sleep_minutes;

/*
 *
 */
void btbee_power(unsigned char on) {
    if (on) {
        digitalWrite(POWER_BTBEE, HIGH);
        delay(2000);
    } else {
        digitalWrite(DISCONNECT_BTBEE, HIGH);
        while (digitalRead(CD_BTBEE) == HIGH) {
            util::blink(4, 500, 100);
        }
        digitalWrite(POWER_BTBEE, LOW);
        digitalWrite(DISCONNECT_BTBEE, LOW);
    }
}

void wake(void) {
    util::delay(1000);
}

void mcu_sleep(void) {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    attachInterrupt(0, wake, RISING);   /* attach to pin 2 */
    sleep_mode();
    detachInterrupt(0);
}

void setup(void) {
    pinMode(RX_WIFLY, INPUT);
    pinMode(TX_WIFLY, OUTPUT);
    pinMode(WIFLY_WAKE, INPUT);
    pinMode(CD_BTBEE, INPUT);

    /*
     * The Seeeduino Stalker allows you turn power
     * off to the Bee port altogether which makes
     * our lives easier.
     */
    pinMode(POWER_BTBEE, OUTPUT);
    pinMode(DISCONNECT_BTBEE, OUTPUT);

    Serial.begin(BTBEE_BAUD_RATE);
    wifly.begin(WIFLY_BAUD_RATE);

    Wire.begin();
    tmp102_init();

    /*
     * We rely on the RTC in the WiFly to know when
     * to wake us up.
     */
    attachInterrupt(0, wake, RISING);   /* attach to pin 2 */
}

void loop(void) {
    unsigned long total_kwh = 0;
    unsigned long spot_ac = 0;
    unsigned long now = 0;
    bool status = false;
    unsigned char retries = MAX_RETRIES;

    /*
     * Try and talk to the inverter.
     */
    while (!status && retries--) {
        delay(5000);

        btbee_power(HIGH);
        if (bt_init()) {
            status = bt_get_status(&total_kwh, &spot_ac, &now);
        }
        btbee_power(LOW);
    }

    /*
     * It looks like the SMA inverter returns the date and
     * time of the last non-zero data read, so if we haven't
     * gotten any fresh data it's quite probable that the
     * sun has gone down (need to see the SMA specs to be sure).
     * Change to checking with the inverter once per hour to
     * save power, and we'll eventually wake up with some
     * new data.
     */
    if (status) {
        if (total_kwh == last_total_kwh &&
            spot_ac == last_spot_ac &&
            now == last_now) {
            sleep_minutes = 60;
        } else {
            last_total_kwh = total_kwh;
            last_spot_ac = spot_ac;
            last_now = now;
            sleep_minutes = 10; /* pvoutput default upload is 10 minutes */

            wifly_init();
            wifly_upload_stats(total_kwh, spot_ac, now, tmp102_get());
        }
    } else {
        sleep_minutes = 10;
    }

    wifly_sleep(sleep_minutes); /* put wifly to sleep */
    mcu_sleep();
}
