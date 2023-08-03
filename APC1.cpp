#include "APC1.h"

// pins used for LCD
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
SoftwareSerial apc1SensorConnection(RX_PIN, TX_PIN);
// struct
apc1Data apcData;
// where data is received before processing
uint8_t rxDataBuffer[DATA_BUFFER_SIZE];

// get response from APC1 (get latest measurement)
static bool __APC1_Read_Data(Stream *s) {
  
  if (s->available() < 0) { // check if we received anything
    return false;
  }

  if (s->peek() == 0x42) { // check for the first byte in the first frame header
    if (s->available() >= 63) {
      s->readBytes(rxDataBuffer, DATA_BUFFER_SIZE); // read 64 bytes
      return true;
    }       
  }

  return false;

}

// inner function to print data to terminal
static void __APC1_Print_Data(apc1Data *data) {

  Serial.println("------Start of measurement-------");
  uint16_t *element = &data->pm1MC; // pointer to the first value of struct
  for (int i = 0; i<STRING_ARRAY_SIZE - 1; i++) {
    Serial.print(strings[i]);
    if (i >= 14) { // if true, then we are at T-comp, RH-Comp, T-raw and RH-raw
      Serial.println((double) *element * 0.1f);
    } else { // everything before first reserved byte (0x20)
      Serial.println(*element);
    }
    if (i == 13) {
      element += 2; // skip the first reserved byte (0x20) 
    } else {
      element++; // otherwise, increment by two bytes (int16_t)
    }
  }
  // its an 8 bit value and this is easier, that using another pointer
  Serial.print("AQI: "); Serial.println(data->aqi);
  Serial.println("------End of measurement-------");

}

// inner function to print data to LCD
// working is the same as __APC1_Print_Data
// it just prints to the LCD
static void __APC1_Print_Data_LCD(apc1Data *data) {

  uint16_t *element = &data->pm1MC;
  char printString[16];
  for (int i = 0; i<18; i++) {
    // clear LCD contents and move cursor to beginning (0, 0)
    lcd.clear();
    lcd.home();
    if (i >= 14) {
      dtostrf(*element * 0.1f, 4, 2, printString); // for floats to string
    } else {
      sprintf(printString, "%u", *element); // regular ints to string
    }
    if (i == 13) {
      element += 2;
    } else {
      element++;
    }
    // print to LCD and wait 2 seconds
    lcd.print(stringsLCD[i]);
    lcd.setCursor(0, 1);
    lcd.print(printString);
    memset(printString, 0, 16);
    delay(2000);
  }
  lcd.clear();
  lcd.home();
  lcd.print("AQI: "); lcd.print((int) data->aqi);

}

// setup everything
void APC1_Setup() {
  
  // set pins
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  
  // set baud rates
  Serial.begin(TERMINAL_BAUD_RATE);
  apc1SensorConnection.begin(APC1_BAUD_RATE);

  // set lcd
  lcd.begin(16, 2);

}

// functipn to get the current measurement data
bool APC1_Get_Data() {
  
  if (apc1SensorConnection.write(getDataCmd, CMD_ARRAY_SIZE) != CMD_ARRAY_SIZE) {
    return false;
  }

  delay(3000);
  return __APC1_Read_Data(&apc1SensorConnection);

}

// function to set APC1 to idle mode
bool APC1_Set_Idle() {

  if (apc1SensorConnection.write(setIdleCmd, CMD_ARRAY_SIZE) != CMD_ARRAY_SIZE) {
    return false;
  }

  return true;

}

// function to set APC1 to measurement mode
bool APC1_Set_Measurement() {

  if (apc1SensorConnection.write(setMeasurementModeCmd, CMD_ARRAY_SIZE) != CMD_ARRAY_SIZE) {
    return false;
  }

  return true;

}

// function to make sense of data received
bool APC1_Process_Data() {

  uint16_t sum = 0;
  for (int i = 0; i<62; i++) {
    sum += rxDataBuffer[i];
  }
  if (sum != (((uint16_t) rxDataBuffer[62] << 8) | rxDataBuffer[63])) { // check, if checksum is correct
    return false;
  }

  uint16_t arr[19];
  int i = 4, index = 0; // i starts at 4 to skip the first four bytes
  uint32_t converted32;
  while (i < DATA_BUFFER_SIZE) {
    if (i < 42) {
      // convert two bytes into the correct 16 bit value
      arr[index++] = ((uint16_t) rxDataBuffer[i] << 8) | rxDataBuffer[i + 1];
      i += 2; // increment by 2 bytes
    } else if (i >= 42 && i < 58) {
      // convert four bytes into the correct 32 bit value (NOT USED)
      converted32 = 0;
	    for (int j = 0; j<4; j++) {
        converted32 = (((uint32_t) converted32 << 8) | rxDataBuffer[i + j]);
      }
      i += 4; // increment by 4 bytes
    } else {
	    break;
    }
  }

  arr[7] = arr[6] - arr[7]; // To determine # of PM0.5 particles subtract field 0x12 from field 0x10. (from the docs)
  arr[8] = arr[6] - arr[8]; // To determine # of PM1.0 particles subtract field 0x14 from field 0x10.
  arr[9] = arr[6] - arr[9]; // To determine # of PM2.5 particles subtract field 0x16 from field 0x10.
  arr[10] = arr[6] - arr[10]; // To determine # of PM5 particles subtract field 0x18 from field 0x10
  arr[11] = arr[6] - arr[11]; // To determine # of PM10 particles subtract field 0x1A from field 0x10.

  memcpy(&apcData, arr, 38);
  apcData.aqi = rxDataBuffer[58];
  apcData.version = rxDataBuffer[60];
  apcData.errorCode = rxDataBuffer[61];

  // assume everything is fine
  return true;

}

// function that the user calls
void APC1_Print() {

  __APC1_Print_Data(&apcData);

}

// function that the user calls
void APC1_Print_LCD() {

  __APC1_Print_Data_LCD(&apcData);

}
