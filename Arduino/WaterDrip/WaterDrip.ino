#include <ArduinoTrace.h>
#define Serial SerialUSB  // For ArduinoTrace

#include <rdm.h>
#include <DMXSerial2.h>

#include <FlashStorage.h>

#include <FPBoard.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>

static int unitNumber = 1;  // Set to -1 to use flash storage; otherwise hard code
int16_t sensorValues[7];
int16_t sensorLow[7] = {32767, 32767, 32767, 32767, 32767, 32767, 32767 };
int16_t sensorHigh[7] = { -32768, -32768, -32768, -32768, -32768, -32768, -32768 }; // Cached sensor values

/*********** Debugging support ************/
#define dbg SerialUSB

#define assert(ex) if (!(ex))  assertFailed(__FILE__, __LINE__, # ex);

void assertFailed(const char *file, int line, const char *ex) {
  dbg.print("Assert failed at ");
  dbg.print(file);
  dbg.print(":");
  dbg.print(line);
  dbg.print(": ");
  dbg.println(ex);
}

/*********** Set pin modes for all pins ************/
void initializePins() {
  // Igniters
  // pinMode(ADJ_P_IGNITERS, OUTPUT); //Unused, leave floating
  for (unsigned int i = 0; i < sizeof(igniterEnables) / sizeof(igniterEnables[0]); i++) {
    digitalWrite(igniterEnables[i], LOW);  // Make sure its off before enabling
    pinMode(igniterEnables[i], OUTPUT);
  }
  digitalWrite(EN_P_IGNITERS, LOW); // Make sure its off before enabling
  pinMode(EN_P_IGNITERS, OUTPUT);

  //pinMode(V_MON_P_IGNITERS, INPUT); // Analog inputs are initialized by analogRead()
  //pinMode(I_MON_P_IGNITERS, INPUT);

  // Solenoids
  digitalWrite(EN_P_SOLENOIDS, LOW); // Make sure its off before enabling
  pinMode(EN_P_SOLENOIDS, OUTPUT);
  for (unsigned int i = 0; i < sizeof(solenoidEnables) / sizeof(solenoidEnables[0]); i++) {
    digitalWrite(solenoidEnables[i], LOW); // Make sure its off before enabling
    pinMode(solenoidEnables[i], OUTPUT);
  }
  //pinMode(I_MON_P_SOLENOIDS, INPUT); // Analog inputs are initialized by analogRead()
  //pinMode(V_MON_P_SOLENOIDS , INPUT);

  // Status LED
  pinMode(LED_STATUS, OUTPUT);

  // OLED Display
  //pinMode(OLED_SDA , OUTPUT); // I2C pins set up by the "Wire" library
  //pinMode(OLED_SCK , OUTPUT);

  // Button pad
  pinMode(BUTTON_LEFT_N , INPUT_PULLUP);
  pinMode(BUTTON_DOWN_N , INPUT_PULLUP);
  pinMode(BUTTON_CENTER_N , INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_N , INPUT_PULLUP);
  pinMode(BUTTON_UP_N , INPUT_PULLUP);

  // DMX I/O
  pinMode(RS485_DIR, OUTPUT);
  pinMode(UART_TX, OUTPUT);
  pinMode(UART_RX , INPUT);

  // Temperature sensors
  pinMode(DS18B20_EXT, OUTPUT);
  pinMode(DS18B20_PCB , INPUT);

  // Unused pins
  pinMode(UNUSED_1 , OUTPUT); // Set unused pins to a known value
  pinMode(UNUSED_2 , OUTPUT);
  pinMode(UNUSED_3 , OUTPUT);
  pinMode(UNUSED_4 , OUTPUT);
  pinMode(UNUSED_5 , OUTPUT);
  pinMode(UNUSED_6 , OUTPUT);
}

/*********** OLED Display ************/
const int SCREEN_WIDTH = 128; // OLED display width, in pixels
const int SCREEN_HEIGHT = 64; // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 1000000UL, 100000UL);

void oledinit() {
  // Initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);             //  2:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0, 24);
  display.println(F("FPDMX2"));
  display.display();
}

/*********** Pressure sensor ************/
Adafruit_BMP280 bmp;
// Use readTemperature(), readPressure()

/*********** External temp sensor ************/
//OneWire oneWire(DS18B20_EXT);
//DallasTemperature temperatureSensors(&oneWire);
DeviceAddress DS18B20_address;


/*********** Buttons ************/
/* Returns a bit string of button states  */
const int LEFT = 1;
const int DOWN = 2;
const int CENTER = 4;
const int RIGHT = 8;
const int UP = 0x10;
int getButtons() {
  return (digitalRead(BUTTON_LEFT_N) == LOW) ? LEFT : 0 |
         (digitalRead(BUTTON_DOWN_N) == LOW) ? DOWN : 0 |
         (digitalRead(BUTTON_CENTER_N) == LOW) ? CENTER : 0 |
         (digitalRead(BUTTON_RIGHT_N) == LOW) ? RIGHT : 0 |
         (digitalRead(BUTTON_UP_N) == LOW) ? UP : 0;
}

/*********** Chip ID ************/
// Returns pointer to 4 32-bit unsigned integers containing serial number
extern uint32_t *getChipId();

/*********** Igniter controls ************/
// Set master enable for igniters (not implemented in logic board currently)
void igniterEnable(bool enable) {
  digitalWrite(EN_P_IGNITERS, enable ? HIGH : LOW );
}

void igniter(int n, bool on) {
  assert((n >= 0) && (n < 4));
  digitalWrite(igniterEnables[n], on ? HIGH : LOW);
}

void allIgniter(bool on) {
  for (int i = 0; i < 4; i++)
    igniter(i, on);
}

float getIgniterVoltage() {
  // TODO: Are the constants here the same as for the solenoids?
  return analogRead(V_MON_P_IGNITERS) * 4.0 * 3300 / 4096; //measurement*1/[voltage divider]*[Vref in mV]/[full scale] = mV
}

float getIgniterCurrent() {
  // TODO: Are the constants here the same as for the solenoids?
  return analogRead(I_MON_P_IGNITERS) * 100.0 * 3300 / (50 * 4096); //measurement*1/[sense resistor]*[Vref in mV]/([Isense gain]*[full scale])
}

/*********** Solenoid controls ************/
void solenoidEnable(bool enable) {
  digitalWrite(EN_P_SOLENOIDS, enable ? HIGH : LOW );
}

void solenoid(int n, bool open) {
  // Set solenoid n (0..15) as open (open=true, air flows) or closed (open=false)
  assert(n >= 0 && n < 16);
  digitalWrite(solenoidEnables[n], open ? HIGH : LOW);
  if (digitalRead(solenoidEnables[n]) != (open ? HIGH : LOW)) {
    dbg.print(n);
    dbg.print("->");
    dbg.print(open);
    dbg.print(", EN=");
    dbg.print(solenoidEnables[n]);
    dbg.print(", state=");
    dbg.println(digitalRead(solenoidEnables[n]));
    TRACE();
    delay(5000);
  }
}

void allSolenoid(bool open) {
  for (int i = 0; i < 16; i++)
    solenoid(i, open);
}

float getSolenoidVoltage() {
  return analogRead(V_MON_P_SOLENOIDS) * 4.0 * 3300 / 4096; //measurement*1/[voltage divider]*[Vref in mV]/[full scale] = mV
}

float getSolenoidCurrent() {
  return analogRead(I_MON_P_SOLENOIDS) * 100.0 * 3300 / (50 * 4096); //measurement*1/[sense resistor]*[Vref in mV]/([Isense gain]*[full scale])
}

/*********** Menu handling ************/
FlashStorage(unit_storage, int);

void setupmenus() {
  //addmenu("Main");
}
void menu() {
#ifdef MENU
  static int currEntry[] = {0, 0, 0}; // Position at each level
  const int nlevels = sizeof(currEntry) / sizeof(currEntry[0]);
  static int currLevel = 0;
#endif

  // Get joystick  state
  static int lastButtons = -1;
  int sw = getButtons();
  // If any buttons were previously pressed, wait until released before doing anything else
  if (lastButtons != 0 && sw != 0)
    return;
  lastButtons = 0;
  // Check if a key is pressed
  if (sw == 0)
    return;
  lastButtons = sw;
#ifdef MENU
  if (sw & RIGHT && currLevel < nlevels - 1) {
    currLevel++;
    currEntry[currLevel] = 0;
  } else if (sw & LEFT && currLevel > 0) {
    currLevel--;
  } else if (sw & UP && currEntry[currLevel] > 0) {
    currEntry[currLevel]--;
  } else if (sw & DOWN) {
    currEntry[currLevel]++;
  }
  dbg.print("Level "); dbg.print(currLevel); dbg.print(", Entry "), dbg.println(currEntry[currLevel]);
#else
  // Simple unit control
  if (sw & UP && unitNumber < 24)
    unitNumber++;
  else if (sw & DOWN && unitNumber > 0)
    unitNumber--;
  unit_storage.write(unitNumber);
#endif
}

// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);

int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}


/*********** Display update ************/
void updateDisplay() {
  // Update status
  static unsigned long lastUpdate = 0;
  static int active = 0;
  unsigned long now = millis();
  if (now - lastUpdate < 100)
    // Only update 10 times/sec
    return;
  //dbg.print("*");
  //dbg.println(freeMemory(), DEC);
  lastUpdate = now;
  unsigned int sw = getButtons();
  char status[20];
  char *sptr = &status[0];
  *sptr++ = (DMXSerial2.noDataSince() < 1000) ? 'D' : '-';
  *sptr++ = (DMXSerial2.noRDMSince() < 1000) ? 'R' : '-';
  *sptr++ = (sw & LEFT) ? 'L' : (sw & RIGHT) ? 'R' : (sw & UP) ? 'U' : (sw & DOWN) ? 'D' : (sw & CENTER) ? 'C' : ' ';
  *sptr++ = 0;
  assert(sptr <= status + sizeof(status));
  display.clearDisplay();
  display.setCursor(0, 12);
  display.print(status);
  display.print(' ');
  display.print(DMXSerial2.getStartAddress(), DEC);
  display.print(' ');
  display.println((active++ % 2 == 1) ? "+" : "-"); // To show we're alive

  DEVICEID id;
  DMXSerial2.getDeviceID(id);
  char buf[9];
  sprintf(buf, "%02x%02x %02x", id[4], id[5], DMXSerial2.readRelative(0));
  display.println(buf);

  display.print(sensorValues[0] / 10.0, 0);
  display.print(" ");
  display.print(sensorValues[1] / 10.0, 0);
  display.print(" ");
  display.println(sensorValues[2] / 248.84, 1); // Convert to inches water above STP

  // Update display
  unsigned long timer = micros();
  display.display();
  unsigned long elapsed = micros() - timer;
  dbg.print("Displ update: ");
  dbg.print(elapsed);
  dbg.println(" micros.");
}

/*********** DMX setup ************/
// see DMXSerial2.h for the definition of the fields of this structure
const uint16_t my_pids[] = {E120_DEVICE_HOURS};
struct RDMSENSOR sensors[7];

struct RDMINIT rdmInit = {
  "fire-pixels.com", // Manufacturer Label
  1, // Device Model ID
  "Arduino RDM Device", // Device Model Label
  20, // footprint
  (sizeof(my_pids) / sizeof(uint16_t)), my_pids,
  sizeof(sensors) / sizeof(sensors[0]), sensors
};

bool8 getSensorValue(uint8_t sensorNr, int16_t *value, int16_t *lowestValue, int16_t *highestValue, int16_t *) {
  if (sensorNr < 7) {
    *value = sensorValues[sensorNr];
    *lowestValue = sensorLow[sensorNr];
    *highestValue = sensorHigh[sensorNr];
    return true;
  }
  return false;
}

void dmxFrameEnd() {
  // Update solenoids
  for (int i = 0; i < 16; i++)
    solenoid(i, DMXSerial2.readRelative(i));
}

// This function was registered to the DMXSerial2 library in the initRDM call.
// Here device specific RDM Commands are implemented.
bool8 processCommand(struct RDMDATA *rdm, uint16_t *nackReason)
{
  byte CmdClass       = rdm->CmdClass;     // command class
  uint16_t Parameter  = rdm->Parameter;     // parameter ID
  bool8 handled = false;

  // This is a sample of how to return some device specific data
  if (Parameter == SWAPINT(E120_DEVICE_HOURS)) { // 0x0400
    if (CmdClass == E120_GET_COMMAND) {
      if (rdm->DataLength > 0) {
        // Unexpected data
        *nackReason = E120_NR_FORMAT_ERROR;
      } else if (rdm->SubDev != 0) {
        // No sub-devices supported
        *nackReason = E120_NR_SUB_DEVICE_OUT_OF_RANGE;
      } else {
        rdm->DataLength = 4;
        unsigned long now = millis();
        rdm->Data[0] = (uint8_t)(now >> 24 & 0xff);
        rdm->Data[1] = (uint8_t)(now >> 16 & 0xff);
        rdm->Data[2] = (uint8_t)(now >> 8 & 0xff);
        rdm->Data[3] = (uint8_t)(now & 0xff);
        handled = true;
      }
    } else if (CmdClass == E120_SET_COMMAND) {
      // This device doesn't support set
      *nackReason = E120_NR_UNSUPPORTED_COMMAND_CLASS;
    }
  } // if

  return handled;
} // processCommand

#define NUMSENSORS 7

void DMXSetup() {
  // Setup sensors
  for (int i = 0; i < 2; i++) {
    sensors[i].type = E120_SENS_TEMPERATURE;
    sensors[i].unit = E120_UNITS_CENTIGRADE;
    sensors[i].prefix =  E120_PREFIX_DECI;
    sensors[i].normalMin = 10;
    sensors[i].lowHighSupported = true;
    sensors[i].recordedSupported = false;
  }
  sensors[0].description = (char *)F("External Temp");
  sensors[0].rangeMin = -55 * 10;
  sensors[0].rangeMax = 125 * 10;
  sensors[0].normalMax = 85 * 10;

  sensors[1].description = (char *)F("Onboard Temp");
  sensors[1].rangeMin = -40 * 10;
  sensors[1].rangeMax = 85 * 10;
  sensors[1].normalMax = 60 * 10;

  sensors[2].type = E120_SENS_PRESSURE;
  sensors[2].unit = E120_UNITS_PASCAL;
  sensors[2].prefix =  E120_PREFIX_NONE;
#define STDPRESSURE 101325
  sensors[2].rangeMin = max(-32768, 30000 - STDPRESSURE);
  sensors[2].rangeMax = 110000 - STDPRESSURE;
#define PASCALPERINCH 248.84
  sensors[2].normalMin = - 1 * PASCALPERINCH;
  sensors[2].normalMax = + 6 * PASCALPERINCH + 1 * PASCALPERINCH;
  sensors[2].lowHighSupported = true;
  sensors[2].recordedSupported = false;
  sensors[2].description = (char *)F("Pressure above std");

  // Voltage sensors
  for (int i = 3; i <= 5; i += 2) {
    sensors[i].type = E120_SENS_VOLTAGE;
    sensors[i].unit = E120_UNITS_VOLTS_DC;
    sensors[i].prefix =  E120_PREFIX_MILLI;
    sensors[i].rangeMin = 0;
    sensors[i].rangeMax = 24000;
    sensors[i].normalMin = 4000;
    sensors[i].normalMax = 5000;
    sensors[i].lowHighSupported = true;
    sensors[i].recordedSupported = false;
  }
  sensors[3].description = (char *)F("Solenoid Voltage");
  sensors[5].description = (char *)F("Igniter Voltage");


  // Current sensors
  for (int i = 4; i <= 6; i += 2) {
    sensors[i].type = E120_SENS_CURRENT;
    sensors[i].unit = E120_UNITS_AMPERE_DC;
    sensors[i].prefix =  E120_PREFIX_MILLI;
    sensors[i].rangeMin = 0;
    sensors[i].rangeMax = 5000;
    sensors[i].normalMin = 0;
    sensors[i].normalMax = 4000;
    sensors[i].lowHighSupported = true;
    sensors[i].recordedSupported = false;
  }
  sensors[4].description = (char *)F("Solenoid Current");
  sensors[6].description = (char *)F("Igniter Current");

  DMXSerial2.init(&rdmInit, processCommand, getSensorValue, RS485_DIR);
  DMXSerial2.attachDMXFrameCallback(dmxFrameEnd);

  uint16_t start = DMXSerial2.getStartAddress();
  dbg.print("Listening on DMX address #"); dbg.println(start);
}
/*********** Arduino setup ************/
void setup() {
  // Debug via USB serial
  //delay(5000);
  dbg.println("FPDMX2");

  // Initialize I/O pins
  initializePins();
  // Setup OLED
  oledinit();


  // Setup pressure sensor
  if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
    dbg.println(F("Could not find a valid BMP280 sensor!"));
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_OFF,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */


  // Initialize off-pcb DS18B20 sensor
//  temperatureSensors.begin();
//  temperatureSensors.getAddress(DS18B20_address, 0);
//  temperatureSensors.setResolution(DS18B20_address, 9);

  // Make sure everything is initialized off
  solenoidEnable(false);
  igniterEnable(false);
  allIgniter(false);
  allSolenoid(false);

  // Initialize ADC to 12-bit resolution (defaults to 10)
  analogReadResolution(12);
  // Initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // Enable solenoids
  solenoidEnable(true);
  // Get unit number from EEPROM
  if (unitNumber == -1)
    unitNumber = unit_storage.read();

  //  DMXSetup();

  dbg.println("FPDMX Ready");

  //  delay(5000);
  //  while (0) {
  //    // Test all solenoids
  //    allSolenoid(true);
  //    delay(5000);
  //    static int ord[] = {5, 4, 3, 2, 6, 7, 0, 1, 9, 8, 15, 14, 10, 11, 12, 13};
  //    for (int i = 0; i < 16; i++) {
  //      solenoid(ord[i], false);
  //      delay(2000);
  //      solenoid(ord[i], true);
  //      delay(1000);
  //    }
  //  }
  //  for (int i = 0; i < 16; i++)
  //    DMXSerial2.write(i + 1, 255);
}

// Dump data in DMX input buffer
void dump()
{
  int prior = 0;
  int rpt = 0;
  dbg.print("[");
  for (int i = 1; i <= 512 ; i++) {
    if (prior != DMXSerial2.read(i))  {
      if (rpt > 0) {
        dbg.print(prior, HEX);
        if (rpt > 1) {
          dbg.print('*');
          dbg.print(rpt, DEC);
        }
        dbg.print(' ');
      }
      prior = DMXSerial2.read(i);
      rpt = 1;
    } else
      rpt++;
  }
  if (rpt > 0) {
    // Print last character
    dbg.print(prior, HEX);
    if (rpt > 1) {
      dbg.print('*');
      dbg.print(rpt, DEC);
    }
    dbg.print(' ');
  }
  dbg.println("]");
}

void sensorupdate()
{
  unsigned long timer = micros();

  const uint32_t updatePeriod[NUMSENSORS] = { 5000, 1000, 1000, 500, 500, 500};
  static uint32_t lastsensorupdate[NUMSENSORS] = { 0, 0, 0, 0, 0, 0};
  for (int i = 0; i < NUMSENSORS; i++) {
    if (millis() - lastsensorupdate[i] > updatePeriod[i]) {
      lastsensorupdate[i] = millis();
      int16_t oldval = sensorValues[i];
      switch (i) {
        case 0:
          //temperatureSensors.requestTemperatures();
          //sensorValues[0] = (int16_t)(temperatureSensors.getTempC(DS18B20_address) * 10);
          break;
        case 1:
          sensorValues[1] = (int16_t)(bmp.readTemperature() * 10);
          break;
        case 2:
          sensorValues[2] = (int16_t)(bmp.readPressure() - 101325);
          break;
        case 3:
          sensorValues[3] = (int16_t)(getSolenoidVoltage());
          break;
        case 4:
          sensorValues[4] = (int16_t)(getSolenoidCurrent());
          break;
        case 5:
          // sensorValues[5] = (int16_t)(getIgniterVoltage());  // If this line is present, then the last solenoid (pin 29) doesn't work
          break;
        case 6:
          //sensorValues[6] = (int16_t)(getIgniterCurrent()); // This one hangs the I2C bus
          break;
      }
      if (sensorValues[i] != oldval) {
        dbg.print("Sensor ");
        dbg.print(i);
        dbg.print(": ");
        dbg.print(sensorValues[i]);
        if (sensorValues[i] != 0) { // Assume 0 is a bad reading
          sensorLow[i] = min(sensorLow[i], sensorValues[i]);
          sensorHigh[i] = max(sensorHigh[i], sensorValues[i]);
        }
        dbg.println("");
      }
      break;  // Only update one sensor
    }
    unsigned long elapsed = micros() - timer;
    if (elapsed > 1000) {
      dbg.print("sensorupdate took ");
      dbg.print(elapsed);
      dbg.print(" micros.");
    }
  }
}

#define WATER 0
#define CAMERA 1

void drip()
{
  solenoid(WATER, 0);
  delay(80);
  solenoid(WATER, 1);
}

void flash()
{
  solenoid(CAMERA, 1);
  delay(100);
  solenoid(CAMERA, 0);
}

/********* Main loop *************/
// Run a cycle of drops
void cycle(int offset[16], int duration) {
  static const int ord[] = {2, 3, 4, 5, 1, 0, 7, 6, 9, 8, 15, 14, 10, 11, 12, 13};
  int start = millis();
  bool active = 1;
  int maxoffset = 0;
  for (int i = 0; i < 16; i++)
    maxoffset = (offset[i] > maxoffset) ? offset[i] : maxoffset;

  while (active) {
    int now = millis() - start;
    active = now < (maxoffset + duration);
    for (int i = 0; i < 16; i++) {
      bool state = (now >= offset[i]) && (now < offset[i] + duration);
      if (i == -1) {
        Serial.print(now);
        Serial.print(": ");
        Serial.print(i);
        Serial.print("=");
        Serial.println(state);
      }
      solenoid(ord[i], !state);
      active = active | state;
    }
    delay(1);
  }
}

void showline(unsigned short line) {
  for (int i = 15; i >= 0; i--) {
    solenoid(ord[i], line & 1);
    line >>= 1;
  }
}

#include "all.h"
int cur = 0;

void showmessage(int len, unsigned short msg[], int period)  {
  for (int i = 0; i < len; i++) {
    delay(period);
    showline(msg[i]);
    if (Serial.available() > 0)
      return;
  }
  showline(0xffff);
}


void loop()
{
  if (Serial.available() > 0) {
    byte in = Serial.read();
    if (in >= '0' && in < ('0' + ndata)) {
      cur = in - '0';
      Serial.print("Src: ");
      Serial.println(names[cur]);
    } else {
      for (int i = 0; i < ndata; i++) {
        Serial.print(i);
        Serial.print(" ");
        Serial.println(names[i]);
      }
    }
  }
  Serial.print("Starting: ");
  Serial.print(cur);
  Serial.print(" ");
  Serial.println(names[cur]);
  digitalWrite(DS18B20_EXT, HIGH);
  showmessage(len[cur], data[cur], step[cur]);
  delay(del[cur]);
}

void loop1()
{
  int offset[16];
  int dur = 50;
  int skew = 10;

  for (int i = 0; i < 16; i++)
    offset[i] = i * skew;
  cycle(offset, dur);


  digitalWrite(DS18B20_EXT, LOW);
  delay(10);
  digitalWrite(DS18B20_EXT, LOW);

  for (int i = 0; i < 16; i++)
    offset[i] = (16 - i) * skew;
  cycle(offset, dur);
  //delay(200);
}

void oldloop()
{
  static int lastButtons = -1;
  int sw = getButtons();
  if (lastButtons != sw) {
    lastButtons = sw;
    dbg.print("Buttons:0x");
    dbg.println(sw, HEX);
    uint32_t *id = getChipId();
    dbg.print("chip id: 0x");
    char buf[33];
    sprintf(buf, "%08lx%08lx%08lx%08lx", id[0], id[1], id[2], id[3]);
    dbg.println(buf);
  }

  menu();

  static uint32_t lastdisplay = 0;
  if (millis() - lastdisplay > 1000) {
    lastdisplay = millis();
    updateDisplay();
  } else {
    sensorupdate(); // Don't update both in the same iteration
  }

  // Display message if no DMX packets
  unsigned long lastPacket = DMXSerial2.noDataSince();
  static unsigned long lastmsg = 0;
  if (lastPacket > 5000 && millis() - lastmsg > 5000) {
    dbg.print("No DMX packets for ");
    dbg.print(lastPacket);
    dbg.println(" ms.");
    lastmsg = millis();
  }

  static bool wasIdentify = false;
  if (DMXSerial2.isIdentifyMode()) {
    // RDM command for identification was sent.
    // Blink the device.
    unsigned long now = millis();
    for (unsigned int i = 0; i < 4; i++) {
      igniter(i, (now / 250) % 4 == i);
    }
    wasIdentify = true;
  } else if (wasIdentify) {
    // Turn off all igniters
    for (unsigned int i = 0; i < 4; i++)
      igniter(i, 0);
    wasIdentify = false;
  }

  // check for unhandled RDM commands
  DMXSerial2.tick();
  // dump DMX data to serial once in a while for debugging
  static uint32_t lastdump = 0;
  if (millis() - lastdump > 10000) {
    lastdump = millis();
    dbg.print("DMX Channel Data: ");
    dump();
  }
}
