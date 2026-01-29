#define DISABLE_FS_H_WARNING  // Disable warning for type File not defined.
#include "SdFat.h"
#if SPI_DRIVER_SELECT == 2  // Must be set in SdFat/SdFatConfig.h
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
#endif
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = 47;
#else   // SDCARD_SS_PIN
// Assume built-in SD is used.
//const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 20;
const uint8_t SOFT_MOSI_PIN = 21;
const uint8_t SOFT_SCK_PIN = 19;

// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)
#endif  // ENABLE_DEDICATED_SPI

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

char line[256];

//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------
// Check for extra characters in field or find minus sign.
char* skipSpace(char* str) {
  while (isspace(*str)) str++;
  return str;
}
//------------------------------------------------------------------------------
bool parseLineOld(char* str) {
  char* ptr;

  // Set strtok start of line.
  str = strtok(str, ",");
  if (!str) return false;

  // Print text field.
  Serial.println(str);

  // Subsequent calls to strtok expects a null pointer.
  str = strtok(nullptr, ",");
  if (!str) return false;

  // Convert string to long integer.
  int32_t i32 = strtol(str, &ptr, 0);
  if (str == ptr || *skipSpace(ptr)) return false;
  Serial.println(i32);

  str = strtok(nullptr, ",");
  if (!str) return false;

  // strtoul accepts a leading minus with unexpected results.
  if (*skipSpace(str) == '-') return false;

  // Convert string to unsigned long integer.
  uint32_t u32 = strtoul(str, &ptr, 0);
  if (str == ptr || *skipSpace(ptr)) return false;
  Serial.println(u32);

  str = strtok(nullptr, ",");
  if (!str) return false;

  // Convert string to double.
  double d = strtod(str, &ptr);
  if (str == ptr || *skipSpace(ptr)) return false;
  Serial.println(d);

  // Check for extra fields.
  return strtok(nullptr, ",") == nullptr;
}


bool parseLine(char* str) {
  char* field;

  // 1. Feld
  field = strtok(str, ",");
  if (!field) return false;
  Serial.println(field);

  // 2. Feld
  field = strtok(nullptr, ",");
  if (!field) return false;
  Serial.println(field);

  // 3. Feld
  field = strtok(nullptr, ",");
  if (!field) return false;
  Serial.println(field);

  // 4. Feld
  field = strtok(nullptr, ",");
  if (!field) return false;
  Serial.println(field);

  // Prüfen, ob es NICHT mehr als 4 Felder gibt
  return strtok(nullptr, ",") == nullptr;
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial) {
    yield();
  }
  Serial.println("Type any character to start");
  while (!Serial.available()) {
    yield();
  }
  // Initialize the SD.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
    return;
  }
  /*
  // Remove any existing file.
  if (sd.exists("ReadCsvDemo.csv")) {
    sd.remove("ReadCsvDemo.csv");
  }
  */
  // Create the file.
  //if (!file.open("ReadCsvDemo.csv", FILE_WRITE)) {
  //  error("open failed");
  //}
  /*
  // Write test data. Test missing CRLF on last line.
  file.print(
      F("abc,123,456,7.89\r\n"
        "def,-321,654,-9.87\r\n"
        "ghi,333,0xff,5.55"));
*/
  // Rewind file for read.
  //file.rewind();

  // Datei NUR LESEN öffnen
  if (!file.open("ReadCsvDemo.csv", O_RDONLY)) {
    error("open for read failed");
  }

  while (file.available()) {
    int n = file.fgets(line, sizeof(line));
    if (n <= 0) {
      error("fgets failed");
    }
    if (line[n - 1] != '\n' && n == (sizeof(line) - 1)) {
      error("line too long");
    }
    if (line[n - 1] == '\n') {
      // Remove new line.
      line[n -1] = 0;
    }
    if (!parseLine(line)) {
      error("parseLine failed");
    }
    Serial.println();
  }
  file.close();
  Serial.println(F("Done"));
}

void loop() {}
