#ifndef UTIL_H
#define UTIL_H

#ifdef DESKTOP
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#endif

class util {
    public:
        util(void);
        ~util(void);

        static void hexdump(unsigned char *buf, unsigned int len, unsigned int dump_width = 16);
        static void perror(const char *str);
        static void msg(const char *fmt, ...);
        static void die(const char *str);
        static void delay(int msec);
        static void blink(int iterations = 1, int msec_on = 100, int msec_off = 100, int led_pin = 8);
        static int strncmp(const char *s1, const char *s2, int len = 0);
        static unsigned long millis(void);
};

#endif /* UTIL_H */
