/*
 * SMA monitor talkey thing. Majority of this code comes from the
 * nanodesmapvmonitor project by Stuart Pittaway, I've just moved
 * stuff around a bit. Thanks, Stuart!
 *

NANODE SMA PV MONITOR

Latest version found at https://github.com/stuartpittaway/nanodesmapvmonitor
Licence
Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
http://creativecommons.org/licenses/by-nc-sa/3.0/

You are free:
    to Share  to copy, distribute and transmit the work
    to Remix  to adapt the work
Under the following conditions:
    Attribution  You must attribute the work in the manner specified by the author or licensor (but not in any way that suggests that they endorse you or your use of the work).
    Noncommercial  You may not use this work for commercial purposes.
    Share Alike  If you alter, transform, or build upon this work, you may distribute the resulting work only under the same or similar license to this one.

Work is copyright Stuart Pittaway, (c)2012.
*/
#ifdef DESKTOP
#include "desktop.h"
#else
#include <Arduino.h>
#include <avr/pgmspace.h>
#endif

#define LEVEL1_MAX_PACKET_SIZE 120
#define LEVEL1_HEADER_SIZE 18

#define SECONDS(n) (n * 1000)

unsigned char sixzeros[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
unsigned char sixff[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
unsigned int FCSChecksum = 0xffff;

unsigned char packet_send_counter = 0;

PROGMEM prog_uint16_t fcstab[256]  = {
  0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
  0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
  0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
  0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
  0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
  0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
  0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
  0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

prog_uchar PROGMEM smanet2packet0[]         = { 0x01, 0x00, 0x00, 0x00 };
prog_uchar PROGMEM smanet2packet1[]         = { 0x80, 0x00, 0x02, 0x00, 0x00 };
prog_uchar PROGMEM smanet2packet2[]         = { 0x80, 0x0E, 0x01, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
prog_uchar PROGMEM SMANET2header[]          = { 0xFF, 0x03, 0x60, 0x65 };
prog_uchar PROGMEM InverterCodeArray[]      = { 0x5c, 0xaf, 0xf0, 0x1d, 0x50, 0x00 };
prog_uchar PROGMEM fourzeros[]              = { 0, 0, 0, 0 };
prog_uchar PROGMEM smanet2acspotvalues[]    = { 0x80, 0x00, 0x02, 0x00, 0x51, 0x00, 0x3f, 0x26, 0x00, 0xFF, 0x3f, 0x26, 0x00, 0x0e };
prog_uchar PROGMEM smanet2totalyieldWh[]    = { 0x80, 0x00, 0x02, 0x00, 0x54, 0x00, 0x01, 0x26, 0x00, 0xFF, 0x01, 0x26, 0x00 };
prog_uchar PROGMEM smanet2packet99[]        = { 0x00, 0x04, 0x70, 0x00 };
prog_uchar PROGMEM smanet2packet_logon[]    = { 0x80, 0x0C, 0x04, 0xFD, 0xFF, 0x07, 0x00, 0x00, 0x00,
                                                0x84, 0x03, 0x00, 0x00, 0xaa, 0xaa, 0xbb, 0xbb };

//Password needs to be 12 bytes long, with zeros as trailing bytes
unsigned char SMAInverterPasscode[] = { '0', '0', '0', '0', 0, 0, 0, 0, 0, 0, 0, 0 };

/*
 * BT addresses are specified in reverse order.
 */
#define BTADDRLEN   6
unsigned char smaBTInverterAddressArray[]   = { 0xde, 0xad, 0xbe, 0xef, 0x0, 0x0 };
unsigned char myBTAddress[]                 = { 0xde, 0xad, 0xbe, 0xef, 0x0, 0x0 };
unsigned long datetime = 0;

/*
 * Offsets in the level 1 packet.
 */
#define PKT_OFF_SOM         0
#define PKT_OFF_LEN1        (PKT_OFF_SOM + 1)
#define PKT_OFF_LEN2        (PKT_OFF_LEN1 + 1)
#define PKT_OFF_CKSUM       (PKT_OFF_LEN2 + 1)
#define PKT_OFF_SRCADDR     (PKT_OFF_CKSUM + 1)
#define PKT_OFF_DSTADDR     (PKT_OFF_SRCADDR + BTADDRLEN)
#define PKT_OFF_CMD         (PKT_OFF_DSTADDR + BTADDRLEN)
#define PKT_OFF_DATASTART   (PKT_OFF_CMD + 2)

#define PKT_SOM             0x7e
#define PKT_LEN(a)          ((unsigned int)(a[PKT_OFF_LEN1] + (a[PKT_OFF_LEN2] << 8)))
#define PKT_CMD(a)          ((unsigned int)(a[PKT_OFF_CMD] + (a[PKT_OFF_CMD+1] << 8)))
#define PKT_DATA(a,n)       (a[PKT_OFF_DATASTART+n])
#define PKT_NETID(a)        (PKT_DATA(a, 4))

#define PKT_0001            ((unsigned int)0x0001)
#define PKT_0002            ((unsigned int)0x0002)
#define PKT_0005            ((unsigned int)0x0005)
#define PKT_000a            ((unsigned int)0x000a)
#define PKT_000c            ((unsigned int)0x000c)

/*
 *
 */
void pkt_send(unsigned char *buf, unsigned int len) {
#ifdef DESKTOP
    util::msg("pkt_send\n");
    util::hexdump(buf, len);
#endif
    while (len--) {
        Serial.write(*buf++);
    }
}

/*
 *
 */
bool get_byte(unsigned char *c) {
    unsigned long timeout = util::millis() + SECONDS(5);

    while (!Serial.available() && util::millis() < timeout) {
        delay(10);
    }

    if (Serial.available()) {
        *c = Serial.read();
        return true;
    }

    return false;
}

/*
 *
 */
void clear_packet(void) {
    unsigned char c;

    while (get_byte(&c));
}

/*
 *
 */
bool pkt_read(unsigned char *buf, unsigned int *len) {
    unsigned char *p = buf;
    unsigned char i;
    unsigned int packet_length;
    unsigned char escape_next = false;

    /*
     * Wait for the start of packet marker.
     */
    while (get_byte(p)) {
        if (*p == 0x7e) {
            p++;
            break;
        }
    }

    if (p == buf) {
        return false;
    }

    /*
     * First four bytes are start of message,
     * length (2 bytes) and checksum.
     */
    for (i = 1; i < 4; i++) {
        if (!get_byte(p)) {
            return false;
        }
        p++;
    }

    packet_length = buf[PKT_OFF_LEN1] + (buf[PKT_OFF_LEN2] << 8);

    /*
     * Pull out sending and receiving addresses
     * and make sure they are correct.
     */
    for (i = 0; i < BTADDRLEN * 2; i++) {
        if (!get_byte(p)) {
            return false;
        }
        p++;
    }

    if (buf[PKT_OFF_CKSUM] != (0x7e ^ buf[PKT_OFF_LEN1] ^ buf[PKT_OFF_LEN2])) {
        util::msg("checksum mismatch\n");
        util::msg("pkt  checksum: 0x%x\n", buf[PKT_OFF_CKSUM]);
        util::msg("calc checksum: 0x%x\n", 0x7e ^ buf[PKT_OFF_LEN1] ^ buf[PKT_OFF_LEN2]);
        util::hexdump(buf, p - buf);
        clear_packet();
        return false;
    }

    if (memcmp(smaBTInverterAddressArray, &buf[PKT_OFF_SRCADDR], BTADDRLEN)) {
        util::msg("failed to match sender address\n");
        util::hexdump(&buf[PKT_OFF_SRCADDR], BTADDRLEN);
        util::hexdump(smaBTInverterAddressArray, BTADDRLEN);
        clear_packet();
        return false;
    }
    if (memcmp(myBTAddress, &buf[PKT_OFF_DSTADDR], BTADDRLEN) &&
        memcmp(sixzeros, &buf[PKT_OFF_DSTADDR], BTADDRLEN) &&
        memcmp(sixff, &buf[PKT_OFF_DSTADDR], BTADDRLEN)) {

        util::msg("failed to match dest address\n");
        util::hexdump(&buf[PKT_OFF_DSTADDR], BTADDRLEN);
        util::hexdump(myBTAddress, BTADDRLEN);
        util::hexdump(sixzeros, BTADDRLEN);
        util::hexdump(sixff, BTADDRLEN);
        clear_packet();
        return false;
    }

    /*
     * Get the command bytes.
     */
    for (i = 0; i < 2; i++) {
        if (!get_byte(p)) {
            clear_packet();
            return false;
        }
        p++;
    }

    /*
     * Pull in the rest of the packet. Note escaped bytes
     * and fix.
     */
    for (i = 0; i < packet_length - PKT_OFF_DATASTART; i++) {
        if (!get_byte(p)) {
            clear_packet();
            return false;
        }

        if (escape_next) {
            *p = *p ^ 0x20;
            escape_next = false;
        } else if (*p == 0x7d) {
            escape_next = true;
        }

        if (!escape_next) {
            p++;
        }
    }

    /*
     * TODO if we receive a level 2 packet, validate
     * it's checksum.
     */

    util::msg("received packet\n");
    util::hexdump(buf, p - buf);

    *len = p - buf;

    return true;
}

/*
 * Wait for packet of type.
 */
bool pkt_wait(unsigned char *buf, unsigned int *len, unsigned int type) {
    while (pkt_read(buf, len)) {
        if (PKT_CMD(buf) == type) {
            return true;
        }
    }
    util::msg("timeout waiting for 0x%x packet\n", PKT_0002);
    return false;
}

unsigned int pkt_header(unsigned char *buf, unsigned int packet_type, unsigned char *dst) {
    unsigned char *p = buf;

    if (packet_send_counter > 75) {
        packet_send_counter = 1;
    }

    FCSChecksum = 0xffff;

    *p++ = 0x7e;
    *p++ = 0; /* length of packet, low byte, set later */
    *p++ = 0; /* length of packet, high byte, set later */
    *p++ = 0; /* checksum, set later */

    memcpy(p, myBTAddress, BTADDRLEN);
    p += BTADDRLEN;

    memcpy(p, dst, BTADDRLEN);
    p += BTADDRLEN;

    *p++ = packet_type & 0xff;
    *p++ = (packet_type >> 8) & 0xff;

    return p - buf;
}

unsigned int pkt_smanet2_byte(unsigned char *buf, unsigned char v) {
    unsigned char *p = buf;

#ifdef DESKTOP
    FCSChecksum = (FCSChecksum >> 8) ^ (fcstab[(FCSChecksum ^ v) & 0xff]);
#else
    FCSChecksum = (FCSChecksum >> 8) ^ pgm_read_word_near(&fcstab[(FCSChecksum ^ v) & 0xff]);
#endif

    if (v == 0x7d || v == 0x7e || v == 0x11 || v == 0x12 || v == 0x13) {
        *p++ = 0x7d;
        *p++ = v ^ 0x20;
    } else {
        *p++ = v;
    }

    return p - buf;
}

unsigned int pkt_smanet2_P(unsigned char *buf, prog_uchar *src, unsigned int num) {
    unsigned char *p = buf;

    while (num--) {
#ifdef DESKTOP
        pkt_smanet2_byte(p++, *src++);
#else
        pkt_smanet2_byte(p++, pgm_read_byte_near(src++));
#endif
    }

    return p - buf;
}

unsigned int pkt_smanet2(unsigned char *buf, unsigned char *src, unsigned int num) {
    unsigned char *p = buf;

    while (num--) {
        pkt_smanet2_byte(p++, *src++);
    }

    return p - buf;
}

void pkt_length_cksum(unsigned char *buf, unsigned int len) {
    buf[PKT_OFF_LEN1] = len & 0xff;
    buf[PKT_OFF_LEN2] = 0;
    buf[PKT_OFF_CKSUM] = PKT_SOM ^ buf[PKT_OFF_LEN1] ^ buf[PKT_OFF_LEN2];
}

void send_init_1(unsigned char *buf, unsigned char netid) {
    unsigned char *p = buf;

    p += pkt_header(p, PKT_0002, smaBTInverterAddressArray);
    p += pkt_smanet2_P(p, smanet2packet99, sizeof(smanet2packet99));
    p += pkt_smanet2(p, &netid, 1);
    p += pkt_smanet2_P(p, fourzeros, sizeof(fourzeros));
    p += pkt_smanet2_P(p, smanet2packet0, sizeof(smanet2packet0));

    pkt_length_cksum(buf, p - buf);

    pkt_send(buf, p - buf);
}

unsigned int pkt_smanet2plus(unsigned char *buf, unsigned char c1, unsigned char c2,
                             unsigned char pc, unsigned char a, unsigned char b,
                             unsigned char c) {
    unsigned char *p = buf;

    *p++ = PKT_SOM; /* exclude from checksum */

    p += pkt_smanet2_P(p, SMANET2header, sizeof(SMANET2header));
    p += pkt_smanet2(p, &c1, 1);
    p += pkt_smanet2(p, &c2, 1);
    p += pkt_smanet2(p, sixff, sizeof(sixff));
    p += pkt_smanet2(p, &a, 1);
    p += pkt_smanet2(p, &b, 1);
    p += pkt_smanet2_P(p, InverterCodeArray, sizeof(InverterCodeArray));
    p += pkt_smanet2_P(p, fourzeros, 1);    /* only need one 0x0, but must be in checksum */
    p += pkt_smanet2(p, &c, 1);
    p += pkt_smanet2_P(p, fourzeros, sizeof(fourzeros));
    p += pkt_smanet2(p, &pc, 1);

    return p - buf;
}

unsigned int pkt_smanet2plus_trailer(unsigned char *buf) {
    unsigned char *p = buf;

    FCSChecksum = FCSChecksum ^ 0xffff;

    *p++ = FCSChecksum & 0x00ff;
    *p++ = (FCSChecksum >> 8) & 0x00ff;
    *p++ = PKT_SOM;

    return p - buf;
}

void send_init_2(unsigned char *buf) {
    unsigned char *p = buf;

    p += pkt_header(p, PKT_0001, sixff);
    p += pkt_smanet2plus(p, 0x09, 0xa0, packet_send_counter, 0, 0, 0);
    p += pkt_smanet2_P(p, smanet2packet1, sizeof(smanet2packet1));
    p += pkt_smanet2_P(p, fourzeros, sizeof(fourzeros));
    p += pkt_smanet2_P(p, fourzeros, sizeof(fourzeros));
    p += pkt_smanet2plus_trailer(p);

    pkt_length_cksum(buf, p - buf);
    pkt_send(buf, p - buf);
    packet_send_counter++;
}

void send_init_3(unsigned char *buf) {
    unsigned char *p = buf;

    p += pkt_header(p, PKT_0001, sixff);
    p += pkt_smanet2plus(p, 0x08, 0xa0, packet_send_counter, 0, 0x03, 0x03);
    p += pkt_smanet2_P(p, smanet2packet2, sizeof(smanet2packet2));
    p += pkt_smanet2plus_trailer(p);
    pkt_length_cksum(buf, p - buf);

    pkt_send(buf, p - buf);

    packet_send_counter++;
}

bool initialiseSMAConnection() {
    unsigned char buf[128];
    unsigned int len;
    unsigned long timeout;

    util::msg("initialiseSMAConnection()\n");

    /*
     * We *must* get a packet before we can begin.
     * The initial packet from the inverter contains
     * the network ID, which we need to use. Without
     * this nothing will work.
     */
    if (!pkt_wait(buf, &len, PKT_0002)) {
        util::msg("timeout waiting for 0x%x packet\n", PKT_0002);
        return false;
    }

    for (timeout = util::millis() + SECONDS(10); util::millis() <= timeout; ) {
        send_init_1(buf, PKT_NETID(buf));

        if (pkt_wait(buf, &len, PKT_0005)) {
            break;
        }
        util::msg("timeout waiting for 0x%x packet\n", PKT_0005);
    }
    if (util::millis() > timeout) {
        util::msg("failed to get response from send_init_1()\n");
        return false;
    }

    for (timeout = util::millis() + SECONDS(10); util::millis() <= timeout; ) {
        send_init_2(buf);
        if (pkt_wait(buf, &len, PKT_0001)) {
            break;
        }
        util::msg("timeout waiting for 0x%x packet\n", PKT_0001);
    }
    if (util::millis() > timeout) {
        util::msg("failed to get response from send_init_2()\n");
        return false;
    }

    send_init_3(buf);

    util::msg("initialisation complete\n");

    return true;
}

unsigned int pkt_password(unsigned char *buf) {
    unsigned char *p = buf;
    unsigned char i;

    for (i = 0; i < sizeof(SMAInverterPasscode); i++) {
        p += pkt_smanet2_byte(p, (SMAInverterPasscode[i] + 0x88) % 0xff);
    }

    return p - buf;
}

void send_logon(unsigned char *buf) {
    unsigned char *p = buf;

    p += pkt_header(p, PKT_0001, sixff);
    p += pkt_smanet2plus(p, 0x0e, 0xa0, packet_send_counter, 0, 0x01, 0x01);
    p += pkt_smanet2_P(p, smanet2packet_logon, sizeof(smanet2packet_logon));
    p += pkt_smanet2_P(p, fourzeros, sizeof(fourzeros));
    p += pkt_password(p);
    p += pkt_smanet2plus_trailer(p);

    pkt_length_cksum(buf, p - buf);
    pkt_send(buf, p - buf);
    packet_send_counter++;
}

bool logonSMAInverter() {
    unsigned long timeout;
    unsigned char buf[128];
    unsigned int len;

    util::msg("logonSMAInverter()\n");

    for (timeout = util::millis() + SECONDS(10); util::millis() <= timeout; ) {
        send_logon(buf);
        if (pkt_wait(buf, &len, PKT_0001)) {
            break;
        }
    }

    if (util::millis() > timeout) {
        util::msg("failed to logon to inverter\n");
        return false;
    }

    util::msg("logon complete\n");

    return true;
}

unsigned long pkt_total_kwh(unsigned char *buf) {
    unsigned char *p = buf;

    p += pkt_header(p, PKT_0001, smaBTInverterAddressArray);
    p += pkt_smanet2plus(p, 0x09, 0xa0, packet_send_counter, 0, 0, 0);
    p += pkt_smanet2_P(p, smanet2totalyieldWh, sizeof(smanet2totalyieldWh));
    p += pkt_smanet2plus_trailer(p);

    pkt_length_cksum(buf, p - buf);
    pkt_send(buf, p - buf);
    packet_send_counter++;

    return p - buf;
}

unsigned long getTotalPowerGeneration() {
    unsigned long value = 0L;
    unsigned long timeout;
    unsigned char buf[128];
    unsigned int len;

    for (timeout = util::millis() + SECONDS(10); util::millis() <= timeout; ) {
        pkt_total_kwh(buf);
        if (pkt_wait(buf, &len, PKT_0001)) {
            break;
        }
    }
    if (util::millis() > timeout) {
        util::msg("failed to get total power generation\n");
        return 0;
    }

    memcpy(&value, &buf[67], 3);
    return value;
}

unsigned int pkt_spot_ac(unsigned char *buf) {
    unsigned char *p = buf;

    p += pkt_header(p, PKT_0001, smaBTInverterAddressArray);
    p += pkt_smanet2plus(p, 0x09, 0xa1, packet_send_counter, 0, 0, 0);
    p += pkt_smanet2_P(p, smanet2acspotvalues, sizeof(smanet2acspotvalues));
    p += pkt_smanet2plus_trailer(p);

    pkt_length_cksum(buf, p - buf);
    pkt_send(buf, p - buf);
    packet_send_counter++;

    return p - buf;
}

unsigned long getInstantACPower(void) {
    unsigned long value = 0L;
    unsigned long timeout;
    unsigned char buf[128];
    unsigned int len;

    for (timeout = util::millis() + SECONDS(10); util::millis() <= timeout; ) {
        pkt_spot_ac(buf);
        if (pkt_wait(buf, &len, PKT_0001)) {
            break;
        }
    }
    if (util::millis() > timeout) {
        util::msg("failed to get spot AC\n");
        return 0;
    }

    memcpy(&datetime, &buf[63], 4);
    memcpy(&value, &buf[67], 3);
    return value;
}

unsigned long getLastDateTime(void) {
    if (!datetime) {
        getInstantACPower();
    }

    /*
     * The time returned from the inverter is UTC.
     * We are +10, so need to adjust for it.
     */
    return datetime += 10L * 60L * 60L;
}
