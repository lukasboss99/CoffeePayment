// CODE RUNS ON ESP32-S3 DEV KIT
// (C) PVG 2025

#include <elapsedMillis.h>  //https://github.com/pfeerick/elapsedMillis
#include <StateMachine.h>   //https://github.com/jrullan/StateMachine
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>
#include <DatabaseOnSD.h>  //https://github.com/divinofire/DatabaseOnSD
#include <SD.h>
#include <ESP32RotaryEncoder.h>  //https://github.com/MaffooClock/ESP32RotaryEncoder

// ------------------------- //

// General
float preis = 0.30;
bool selectionMenu = 0;
int lastSelection = -1;
bool restartRequested = false;
int i = 0;
bool foundUID = false;
int nutzerNummer = 9999;
float saldo = 9999.0;
float ladebetrag = 9999.0;
int counter = 0;
bool machine_ready = 0;
bool bare_LED;
bool LED_var;
//String name = "UNBEKANNT";
// Optokoppler
#define PIN_PC817 45
//Relais
#define PIN_RELAIS 48
// Timer
elapsedMillis timerRFID;
elapsedMillis monitor;
elapsedMillis debounce;
elapsedMillis stateJump;
elapsedMillis bezug;
elapsedMillis aufladebestaetigung;
elapsedMillis t_relais;
elapsedMillis powerLED;
elapsedSeconds downLED;
// StateMachine
int current_state = 0;
StateMachine machine = StateMachine();
State* S1 = machine.addState(&state1);  // Idle, bitte keytag auflegen
State* S2 = machine.addState(&state2);  // Modus auswählen
State* S3 = machine.addState(&state3);  // Bezug
State* S4 = machine.addState(&state4);  // Aufladen
State* S5 = machine.addState(&state5);  // Aufladen bestätigen
// SPI
SPIClass spiBus(HSPI);  // SPI 1 für RFID und Display
SPIClass spiSD(FSPI);   // SPI 1 für RFID und Display
// Display
#define TFT_CS 15
#define TFT_DC 16
#define TFT_RST 7
Adafruit_ST7735 tft = Adafruit_ST7735(&spiBus, TFT_CS, TFT_DC, TFT_RST);
#define COLOR_ON ST77XX_BLUE
#define COLOR_OFF_RING ST77XX_BLUE
#define COLOR_OFF_FILL ST77XX_BLACK
// Position und Größe der virtuellen LED auf dem Display
const int VLED_X = 140;  // X-Position
const int VLED_Y = 10;   // Y-Position
const int VLED_R = 5;    // Radius
// SD Card
#define SD_CS 47
#define SD_CLK 19
#define SD_MISO 20
#define SD_MOSI 21
MyTable db("keytags.csv");
MyTable dbcopy("backup.csv");
int numRows;
// RFID
#define RFID_CS 4
MFRC522DriverPinSimple rfid_cs_pin(RFID_CS);

MFRC522DriverSPI driver{
  rfid_cs_pin,
  spiBus,
  SPISettings(2000000, MSBFIRST, SPI_MODE0)  // 2 MHz
};

MFRC522 mfrc522{ driver };

bool cardPresent = 0;
uint64_t uidDec = 0;
// Encoder
bool ok_button;
long rawValue = 0;
float scaledValue = 0.0;
unsigned long button_duration;
const uint8_t DI_ENCODER_A = 41;
const uint8_t DI_ENCODER_B = 40;
const int8_t DI_ENCODER_SW = 42;
RotaryEncoder rotaryEncoder(DI_ENCODER_A, DI_ENCODER_B);

// ------------------------ //

void setup() {
  // Serial Monitor
  //Serial.begin(115200);
  //Serial.println("Serial i.O.");

  pinMode(SD_CS, OUTPUT);
  pinMode(TFT_CS, OUTPUT);
  pinMode(RFID_CS, OUTPUT);

  digitalWrite(SD_CS, HIGH);
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(RFID_CS, HIGH);

  spiBus.begin(36, 37, 35);  // Display and RFID, SPI 1
  spiSD.begin(SD_CLK, SD_MISO, SD_MOSI);
  // SD Card
  if (!SD.begin(SD_CS, spiSD, 4000000)) {
    //Serial.println("SD-Karte konnte nicht initialisiert werden");
    while (true) delay(1000);
  }
  //Serial.println("SD-Karte erkannt");
 
  // Rotary Encoder
  rotaryEncoder.setEncoderType(EncoderType::HAS_PULLUP);
  rotaryEncoder.setBoundaries(-1000, 1000, false);
  rotaryEncoder.onTurned(&knobCallback);
  rotaryEncoder.begin();
  pinMode(DI_ENCODER_SW, INPUT);

  //Optokoppler
  pinMode(PIN_PC817, INPUT_PULLUP);
  //Relais
  pinMode(PIN_RELAIS, OUTPUT);
  digitalWrite(PIN_RELAIS, LOW);
  // Database
  //db.printSDstatus();  //[optional] print the initialization status of SD card
  //db.emptyTable();     //[optional] empty table content (make sure to call begin(rowN, colN) after emptying a table) // you could always add more rows.
  db.begin(1, 4);  //[optional] initialize an empty table with x rows and y columns (has no effect if table is not empty)
  //db.writeCell(0, 0, "ID_LOW");
  //db.writeCell(0, 1, "ID_HIGH");
  //db.writeCell(0, 2, "SALDO");
  //db.writeCell(0, 3, "COUNTER");
  /************************************************************************************************************************************************************
            /////////////////////////////////////////////////////////////Update copy csv/////////////////////////////////////////////////////////////////////
  ************************************************************************************************************************************************************/
  //Backup CSV Datei
  dbcopy.begin(1, 4);
  //dbcopy.writeCell(0, 0, "ID_LOW");
  //dbcopy.writeCell(0, 1, "ID_HIGH");
  //dbcopy.writeCell(0, 2, "SALDO");
  //dbcopy.writeCell(0, 3, "COUNTER");
  /************************************************************************************************************************************************************
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ************************************************************************************************************************************************************/
  // Display
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);  // Querformat: 160x128
  tft.fillScreen(ST77XX_BLACK);
  // RFID
  mfrc522.PCD_Init();
  // State Machine
  S1->addTransition(&transitionS1S2, S2);  // Vom Idle zum Menü
  S2->addTransition(&transitionS2S3, S3);  // Vom Menü zum Bezug
  S2->addTransition(&transitionS2S4, S4);  // Vom Menü zum Laden
  S3->addTransition(&transitionS3S2, S2);  // Wenn Bezug abgeschlossen zurück zum Menü
  S4->addTransition(&transitionS4S5, S5);  // Vom Aufladen zum Bestätigen
  S5->addTransition(&transitionS5S2, S2);  // Vom Aufladen bestätigen zurück zum Menü
  // Transitionen von allen Zuständen zurück zu S1
  S2->addTransition(transitionS2S1, S1);
  S3->addTransition(transitionS3S1, S1);
  S4->addTransition(transitionS4S1, S1);
  S5->addTransition(transitionS5S1, S1);
}

// ------------------------ //

void loop() {
  if (!cardPresent) {  // Beim Abziehen der Karte: Transition auf S1 egal von wo
    restartRequested = true;
  } else {
    restartRequested = false;
  }
  machine.run();
  readRFID();
  updateButton();
  machineReady();
  /*
  if (monitor > 250) {
    //Serial.println(uidDec);
    //Serial.println(cardPresent);
    // Serial.print("button: ");
    // Serial.println(ok_button);
    // Serial.print("selectionMenu: ");
    // Serial.println(selectionMenu);
    monitor = 0;
  }
  */
}