/*
 * Temperature code from the logger example available at:
 * http://seeedstudio.com/wiki/Seeeduino_Stalker_v2.0
 *
 * Logger code available from:
 * http://seeedstudio.com/wiki/File:Stlker_logger_AM06_Serial.zip
 */
#include "tmp102.h"

#define TMP102_I2C_ADDRESS 72 /* This is the I2C address for our chip.
This value is correct if you tie the ADD0 pin to ground. See the datasheet for some other values. */
/*
 Table 1. Pointer Register Byte
 P7 P6 P5 P4 P3 P2    P1 P0
 0  0  0  0  0  0  Register Bits
 
 P1 P0 REGISTER
 0  0  Temperature Register (Read Only)
 0  1  Configuration Register (Read/Write)
 1  0  T LOW Register (Read/Write)
 1  1  T HIGH Register (Read/Write)
 */


void tmp102_init(void) {
}


float tmp102_get(void) {
    unsigned char firstbyte; 
    unsigned char secondbyte; //these are the bytes we read from the TMP102 temperature registers
    unsigned int complement;// = 0xe70 - 1
    int tmp102_val;
    float convertedtemp;

    //float correctedtemp; 
    // The sensor overreads (?) 

    /* Reset the register pointer (by default it is ready to read temperatures)
    You can alter it to a writeable register and alter some of the configuration - 
    the sensor is capable of alerting you if the temperature is above or below a specified threshold. */

    Wire.beginTransmission(TMP102_I2C_ADDRESS); //Say hi to the sensor. 
    Wire.write((uint8_t)0x00);
    Wire.endTransmission();
    Wire.requestFrom(TMP102_I2C_ADDRESS, 2);
    Wire.endTransmission();

    firstbyte = (Wire.read()); 
    /*read the TMP102 datasheet - here we read one byte from
    each of the temperature registers on the TMP102*/
    secondbyte = (Wire.read()); 
    /*The first byte contains the most significant bits, and 
    the second the less significant */
    tmp102_val = ((firstbyte) << 4);  
    /* MSB */
    tmp102_val |= (secondbyte >> 4);    
    /* LSB is ORed into the second 4 bits of our byte.
    Bitwise maths is a bit funky, but there's a good tutorial on the playground*/

    /*

    Serial.println();
    Serial.print("complement1 = 0x");
    Serial.println(complement,HEX);

    complement ^= 0xfff;

    Serial.println();
    Serial.print("complement2 = 0x");
    Serial.println(complement,HEX);
    */

    //negative temperature
    if (tmp102_val & 0x800) {
        complement = tmp102_val - 1;
        complement ^= 0xfff;
        convertedtemp = complement * 0.0625 * (-1);
    } else {
        convertedtemp = tmp102_val * 0.0625;
    }
    //correctedtemp = convertedtemp - 5; 
    /* See the above note on overreading */
    return convertedtemp - 5;
}
