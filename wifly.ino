#ifdef DESKTOP
#include "desktop.h"
#endif

/*
 * Our POST header. We only send to pvoutput here.
 */
const char http_post[] PROGMEM =
    "POST /service/r2/addstatus.jsp HTTP/1.1\n"
    "Host: pvoutput.org\n"
    "User-Agent: dfinvrelay/1.0\n"
    "X-Pvoutput-Apikey: your_pvoutput_api_key_goes_here\n"
    "X-Pvoutput-SystemId: your_pvoutput_system_key_goes_here\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "Content-Length: "
;

void wifly_clear(void) {
    while (wifly.available()) {
        wifly.read();
        util::delay(10);
    }
}

void wifly_send(int v) {
    wifly.print(v, DEC);
}
void wifly_send(unsigned int v) {
    wifly.print(v, DEC);
}

void wifly_send(unsigned char c) {
    wifly.write(c);
}

void wifly_send(const char *str) {
    while (*str) {
        wifly_send((unsigned char)*str++);
    }
}

void wifly_send_flash(const char *s) {
    char c;

    while ((c = pgm_read_byte(s++)) != 0) {
        wifly_send((unsigned char)c);
    }
}

void wifly_send_bin(unsigned char *buf, unsigned char len) {
    unsigned char i;

    for (i = 0; i < len; i++) {
        wifly_send(buf[i]);
    }
}

bool wifly_wait(const char *str, unsigned long ms) {
    const char *p = str;
    char c;
    unsigned long timeout = util::millis() + ms;

    while (*p && util::millis() < timeout) {
        if (wifly.available()) {
            c = wifly.read();
            if (*p == c) {
                p++;
                timeout = util::millis() + ms;
            } else {
                p = str;
            }
        }
        util::delay(50);
    }

    return *p == 0 ? true : false;
}

void wifly_cmd_mode(void) {
    wifly_send("$$$");
    wifly_wait("CMD", 5000);
	wifly_send("\r");
	wifly_wait("CMD", 5000);
}

void wifly_data_mode(void) {
    wifly_send("exit\r");
}

/*
 * Format POST as per pvoutput specs.
 */
void wifly_upload_stats(unsigned long total_kwh, unsigned long spot_ac, time_t now, float temp) {
    char data_buf[128] = { 0 };
    String post_data = String("d=");

    int temp_integer_part;
    int temp_fractional_part;

    temp_integer_part = (int)temp;
    temp_fractional_part = (int)((temp - temp_integer_part) * 10.0);    /* 1 decimal place good enough */

    post_data += year(now);
    if (month(now) < 10) {
        post_data += "0";
    }
    post_data += month(now);
    if (day(now) < 10) {
        post_data += "0";
    }
    post_data += day(now);

    post_data += "&t=";
    if (hour(now) < 10) {
        post_data += "0";
    }
    post_data += hour(now);
    post_data += ":";
    if (minute(now) < 10) {
        post_data += "0";
    }
    post_data += minute(now);

    post_data += "&v1=";
    post_data += total_kwh;
    post_data += "&v2=";
    post_data += spot_ac;
    post_data += "&v5=";
    post_data += temp_integer_part;
    post_data += ".";
    post_data += temp_fractional_part;
    post_data += "&c1=1";

    /*
     * 2nd argument is size of 1st argument, not number
     * of bytes to copy.
     */
    post_data.toCharArray(data_buf, post_data.length()+1);

    wifly_send_flash(http_post);
    wifly_send(post_data.length());
    wifly_send("\n\n");
    wifly_send(data_buf);
    wifly_send("\n\n");

    wifly_wait("CLOS*", 1000);
    wifly_clear();
}

void wifly_init(void) {
    wifly_clear();
}

void wifly_sleep(unsigned char minutes) {
    wifly_cmd_mode();
    wifly_send("set sys wake ");
    wifly_send(minutes * 60);
    wifly_send("\r");

    wifly_wait("AOK", 5000);
    wifly_send("sleep\r");
}
