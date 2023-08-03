
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <string.h>
#include <stdint.h>

#define STRING_ARRAY_SIZE 19
#define CMD_ARRAY_SIZE 7
#define DATA_BUFFER_SIZE 64
#define TX_PIN 3
#define RX_PIN 2
#define APC1_BAUD_RATE 9600
#define TERMINAL_BAUD_RATE 115200

// struct where all the data is kept
typedef struct {
    uint16_t pm1MC, pm25MC, pm10MC,
             pm1Air, pm25Air, pm10Air,
             particles03, particles05, particles1,
             particles25, particles50, particles10,
             tvoc, eCO2, reserved1,
             tComp, rhComp, tRaw,
             rhRaw;
    uint8_t aqi, reserved2, version, errorCode;
} apc1Data;

// function prototypes
void APC1_Setup();
bool APC1_Get_Data();
bool APC1_Process_Data();
void APC1_Print();
void APC1_Print_LCD();
bool APC1_Set_Idle();
bool APC1_Set_Measurement();

// get current measurement data
const uint8_t getDataCmd[CMD_ARRAY_SIZE] = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71};
// set to idle mode
const uint8_t setIdleCmd[CMD_ARRAY_SIZE] = {0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73};
// set to measurement mode
const uint8_t setMeasurementModeCmd[CMD_ARRAY_SIZE] = {0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74};

// strings that are used for the terminal
static const char *strings[STRING_ARRAY_SIZE] = {
  "PM1.0 Mass concentration: ",
  "PM2.5 Mass concentration: ",
  "PM10 Mass concentration: ",
  "PM1.0 In air: ",
  "PM2.5 In air: ",
  "PM10 In air: ",
  "# particles > 0.3nm: ",
  "# particles > 0.5nm: ",
  "# particles > 1.0nm: ",
  "# particles > 2.5nm: ",
  "# particles > 5.0nm: ",
  "# particles > 10nm: ",
  "TVOC: ",
  "eCO2: ",
  "T-comp: ",
  "RH-comp: ",
  "T-raw: ",
  "RH-raw: ",
  "AQI: "
};

// strings that are used for the LCD display
static const char *stringsLCD[STRING_ARRAY_SIZE] = {
  "PM1.0 Mass con.:",
  "PM2.5 Mass con.:",
  "PM10 Mass con.:",
  "PM1.0 In air: ",
  "PM2.5 In air: ",
  "PM10 In air: ",
  "# par. > 0.3nm:",
  "# par. > 0.5nm:",
  "# par. > 1.0nm:",
  "# par. > 2.5nm:",
  "# par. > 5.0nm:",
  "# par. > 10nm:",
  "TVOC:",
  "eCO2:",
  "T-comp:",
  "RH-comp:",
  "T-raw:",
  "RH-raw:",
  "AQI:"
};