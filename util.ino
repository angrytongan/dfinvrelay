#include "util.h"

util::util(void) {
}

util::~util(void) {
}

void util::hexdump(unsigned char *buf, unsigned int len, unsigned int dump_width) {
#ifdef DEBUG
#ifdef DESKTOP
	unsigned char *p;
	unsigned char *q;
	unsigned char *e = buf + len;
	unsigned char i;

	struct timeval tp;

	if (gettimeofday(&tp, NULL) != -1) {
		printf("%s", ctime(&tp.tv_sec));
	}

	for (p = q = buf; p <= e; ) {
		if (p == e || (p - q) == (int)dump_width) {
			for (i = 0; i < dump_width - (p - q); i++) {
				printf("   ");
			}

			printf(": ");
			for (; q < p; q++) {
				printf("%c", (*q >= 0x20 && *q <= 0x7e) ? *q : '.');
			}
			printf("\n");
			q = p;

			if (p == e) {
				break;
			}
		} else {
			printf("%02x ", *p);
			p++;
		}
	}
	printf("\n");
    fflush(stdout);
#endif /* DESKTOP */
#endif /* DEBUG */
}

void util::perror(const char *str) {
#ifdef DEBUG
#ifdef DESKTOP
    ::perror(str);
#endif /* DESKTOP */
#endif /* DEBUG */
}
void util::msg(const char *fmt, ...) {
#ifdef DEBUG
#ifdef DESKTOP
    char buffer[512] = { 0 };
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buffer, 512, fmt, ap);
    va_end(ap);

    printf("%s", buffer);
    fflush(stdout);
#endif /* DESKTOP */
#endif /* DEBUG */
}

void util::die(const char *str) {
#ifdef DEBUG
    if (str) {
        perror(str);
    }
#endif
#ifdef DESKTOP
	exit(1);
#else
    for (;;) {
        util::blink(10, 200, 200);
    }
#endif
}

void util::blink(int iterations, int msec_on, int msec_off, int led_pin) {
#ifdef DESKTOP
    char spinner[] = "\\|-/";
    static char *c = spinner;
    unsigned long timeout = util::millis() + iterations * (msec_on + msec_off);

    while (util::millis() < timeout) {
        util::msg("%c\r", *c++);
        util::delay(msec_on);
        if (!*c) {
            c = spinner;
        }

        util::msg(" \r");
        util::delay(msec_off);
    }
#else
    digitalWrite(led_pin, LOW);

    while (iterations--) {
        digitalWrite(led_pin, HIGH);
        util::delay(msec_on);
        digitalWrite(led_pin, LOW);
        util::delay(msec_off);
    }

    digitalWrite(led_pin, LOW);
#endif
}

void util::delay(int msec) {
#ifdef DESKTOP
    struct timeval tv = { 0 };

    tv.tv_usec = msec * 1000;

    select(0, NULL, NULL, NULL, &tv);
#else
    ::delay(msec);
#endif
}

/*
 * XXX Uh, does Arduino have a strncmp?
 */
int util::strncmp(const char *s1, const char *s2, int len) {
    while (*s1 == *s2 && *s1 != 0 && *s2 != 0 && --len) {
        s1++;
        s2++;
    }

    return len == 0 && *s1 == *s2 ? 0 : /* XXX bogus */
           *s1 == 0 && *s2 == 0 ? 0 :
           *s1 == 0 ? -1 :
           1;
}

unsigned long util::millis(void) {
#ifdef DESKTOP
    static struct timeval start = { 0 };
    struct timeval now = { 0 };

    if (start.tv_sec == 0 && start.tv_usec == 0) {
        gettimeofday(&start, NULL);
    }

    gettimeofday(&now, NULL);

    return ((now.tv_sec - start.tv_sec) * 1000 * 1000 +
           (now.tv_usec - start.tv_usec)) / 1000;
#else
    return ::millis();
#endif
}

#ifdef UNITTEST
int main(void) {
	unsigned char buf[256];
	unsigned int i;

    util::msg("testing %s %d %f\n", "foobar", 1, 12.34);

	for (i = 0; i < 256; i++) {
		buf[i] = i;
	}

	util::hexdump(buf, 256);

    util::msg("%d\n", util::strncmp("foo1", "foo2")); /* 1 */
    util::msg("%d\n", util::strncmp("foo1", "foo2", 2)); /* 0 */
    util::msg("%d\n", util::strncmp("foo1", "foo2", 4)); /* 1 */
}
#endif /* MAIN */
