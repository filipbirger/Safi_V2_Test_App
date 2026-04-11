#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include <stdbool.h>
#include "Particle.h"

//confirm which Particle pin corresponds to P0.07 on the schematic
#define DS18B20_PIN D2

//TRANSACTION SEQUENCE: Initialization, ROM Command, DS18B20 function command

//ROM
//the last 8 bits of the 64-bit code in ROM contains the family code, 0x28
//the next 48 bits contain a unique serial number
//the next 8 bits contain the CRC check from the first 56 bits

//ROM commands:
#define DS18B20_CMD_SKIP_ROM    0xCC  //single device on bus, skip ROM matching
#define DS18B20_CMD_READ_ROM    0x33  //read 64-bit ROM code (single device only)
#define DS18B20_CMD_MATCH_ROM   0x55  //address a specific device by ROM code

//SCRATCHPAD
//9 bytes: Temp LSB, Temp MSB, THYST, TOS, reserved, reserved, reserved, reserved, CRC

//Function commands:
#define DS18B20_CMD_CONVERT     0x44  //start temperature conversion
#define DS18B20_CMD_READ_SCRATCHPAD  0xBE  //read 9 bytes from scratchpad

//conversion times in milliseconds for each resolution
#define DS18B20_CONVERSION_MS   750

//one-wire timing parameters in microseconds (us)
#define OW_RESET_PULSE_US       480  //pull low for reset
#define OW_PRESENCE_WAIT_US     70   //wait after releasing reset before sampling
#define OW_PRESENCE_TIMEOUT_US  240  //max time to wait for presence pulse
#define OW_WRITE_SLOT_US        60   //total write slot duration
#define OW_WRITE_1_LOW_US       6    //pull low time for writing a 1
#define OW_WRITE_0_LOW_US       60   //pull low time for writing a 0
#define OW_READ_INIT_US         3    //pull low to start a read slot
#define OW_READ_SAMPLE_US       10   //time after releasing before sampling
#define OW_READ_SLOT_US         60   //total read slot duration
#define OW_RECOVERY_US          1    //minimum recovery time between slots

/* Temperature thresholds */
#define LOW_REFERENCE_TEMP      25.0f
#define MED_REFERENCE_TEMP      65.0f
#define HIGH_REFERENCE_TEMP     85.0f
#define SUCCESS_ACCURACY        0.5f

/* Function prototypes */
bool  DS18B20_Init(void);          //checks device is present on the bus
bool  DS18B20_Present(void);       //sends reset, returns true if presence pulse detected
float DS18B20_ReadTempC(void);     //triggers conversion and reads temperature in °C
bool  DS18B20_ReadRaw(int16_t *raw); //reads raw 16-bit scratchpad value, no conversion to float

#endif