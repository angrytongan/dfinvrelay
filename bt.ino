#include "sma.h"
#include "relay.h"
#include "util.h"

#define SECONDS(n)  (n * 1000)

void bt_clear(void) {
    while (Serial.available()) {
        Serial.read();
        delay(10);
    }
}

bool bt_wait_string(char *str, int len, unsigned long ms) {
    char *p = str;
    char last = 0;
    char last_prior = 0;
    unsigned long timeout = util::millis() + ms;

    while (util::millis() <= timeout) {
        while (Serial.available()) {
            last = *p++ = Serial.read();

            if (last == '\n' && last_prior == '\r') {
                *(p - 2) = 0;
                return p - 2 - str;
            }
            last_prior = last;

            if (p - str == len - 1) {
                *p = 0;
                return true;
            }
            timeout = util::millis() + ms;
        }
    }
    *p = 0;
    return p - str;
}

void bt_send(const char *str) {
    util::msg("sending: %s\n", str);

    Serial.write("\r\n+");
    Serial.write((char*)str);
    Serial.write("\r\n");
}

char wait_state(char state) {
    char str[128] = { 0 };

    /*
     * XXX Don't need to test the entire string - just wait
     * for BTSTATE: and get the first character after that.
     * XXX These strings should go in flash.
     */
    if (bt_wait_string(str, sizeof(str), SECONDS(5))) {
        util::msg("rcvd: '%s'\n", str);
        if (!util::strncmp(str, "+BTSTATE:0", 10)) {
            state = 0;
        } else if (!util::strncmp(str, "+BTSTATE:1", 10)) {
            state = 1;
        } else if (!util::strncmp(str, "+BTSTATE:2", 10)) {
            state = 2;
        } else if (!util::strncmp(str, "+BTSTATE:3", 10)) {
            state = 3;
        } else if (!util::strncmp(str, "+BTSTATE:4", 10)) {
            state = 4;
        } else if (!util::strncmp(str, "+RTINQ=0,80,25,1F,13,8F", 23)) {
            state = 5;
        } else if (!util::strncmp(str, "~", 1)) { /* getting data packets */
            state = 4;
        }
    }
    return state;
}

bool bt_connected(void) {
    return digitalRead(CD_BTBEE) == HIGH;
}

bool pair(void) {
    char state = -1;
    unsigned long timeout;

    bt_clear();

    /*
     * Wait for device to settle before doing anything. Retain
     * the last state that we achieved - if we timeout then we
     * should be in a known state.
     */
    for (timeout = util::millis() + SECONDS(5); util::millis() < timeout; ) {
        state = wait_state(state);
    }

    if (state == -1) {
        /*
         * Nothing in timeout period. We are either already
         * connected and the inverter is waiting for us, or
         * we're in command mode.
         */
        if (bt_connected()) {
            return true;
        }

        /*
         * Assume we're in command mode.
         */
        state = 1;
    }

    timeout = util::millis() + SECONDS(10);
    while (util::millis() < timeout) {
        switch (state) {
            case 1:
                bt_send("INQ=1");
                timeout = util::millis() + SECONDS(30);
                break;

            case 4:
                return true;

            case 5:
                bt_send("CONN=0,0,0,0,0,0");    /* XXX include this from sma.ino */
                timeout = util::millis() + SECONDS(10);
                break;

            default:
                break;
        }

        state = wait_state(state);
    }

    /*
     * So we aren't receiving data messages, no response
     * from command messages either, or we didn't get any
     * thing at all from the inverter or BT module. I
     * think we can consider that a fail.
     */
    return false;
}

bool bt_init(void) {
    return pair() && initialiseSMAConnection() && logonSMAInverter();
}

bool bt_get_status(unsigned long *total_kwh, unsigned long *spot_ac, unsigned long *now) {
    *spot_ac = getInstantACPower();
    *total_kwh = getTotalPowerGeneration();
    *now = getLastDateTime();

    return true;
}
