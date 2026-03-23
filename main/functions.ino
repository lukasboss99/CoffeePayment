//-------------------------------------------//
void readRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    uidDec = 0;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidDec = (uidDec << 8) | mfrc522.uid.uidByte[i];
    }
    timerRFID = 0;
    cardPresent = 1;
  }
  if (timerRFID > 2500) { // 500 reicht hier eigentlich wenn nicht andere stellen im Code blockieren
    cardPresent = 0;
  }
}
//-------------------------------------------//
void drawArrow(int textX, int textY) {
  int arrowTipX = textX - 10;
  int arrowTipY = textY + 12;
  int size = 8;

  tft.fillTriangle(
    arrowTipX, arrowTipY,
    arrowTipX - size, arrowTipY - size,
    arrowTipX - size, arrowTipY + size,
    ST77XX_RED);
}
//-------------------------------------------//
void clearArrow(int textX, int textY) {
  int arrowTipX = textX - 10;
  int arrowTipY = textY + 12;
  int size = 8;

  // Gleiche Koordinaten wie drawArrow, aber mit Hintergrundfarbe
  tft.fillTriangle(
    arrowTipX, arrowTipY,
    arrowTipX - size, arrowTipY - size,
    arrowTipX - size, arrowTipY + size,
    ST77XX_BLACK);  // Hintergrundfarbe
}
//-------------------------------------------//
void knobCallback(long value) {
  scaledValue = 0.0;
  scaledValue = value * 5.0;
  rawValue = scaledValue / 5;
  if (current_state == 4) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.print("SALDO");
    tft.setCursor(40, 5);
    tft.print(nutzerNummer);
    tft.setTextSize(2);
    tft.setCursor(5, 20);
    tft.print(saldo);
    tft.setCursor(70, 20);
    tft.print("EUR");
    tft.setTextSize(2);
    tft.setCursor(10, 60);
    tft.print("Aufladen um:");
    tft.setCursor(10, 80);
    tft.print(scaledValue);
    tft.setCursor(80, 80);
    tft.print("EUR");
  }
}
//-------------------------------------------//
void updateArrow(int selectionMenu) {
  if (selectionMenu != lastSelection) {
    // Alten Pfeil löschen
    if (lastSelection == 0) clearArrow(30, 60);
    else if (lastSelection == 1) clearArrow(30, 95);

    // Neuen Pfeil zeichnen
    if (selectionMenu == 0) drawArrow(30, 60);
    else if (selectionMenu == 1) drawArrow(30, 95);

    lastSelection = selectionMenu;
  }
}
//-------------------------------------------//
void updateButton() {
  if (debounce > 20) {
    ok_button = !digitalRead(DI_ENCODER_SW);
    debounce = 0;
  }
}
//-------------------------------------------//
void machineReady() {
  bare_LED = !digitalRead(PIN_PC817);
  Serial.print("bare_LED: ");
  Serial.println(bare_LED);
  Serial.print("LED_var: ");
  Serial.println(LED_var);
  if (bare_LED) {    // Power LED Raw Signal
    LED_var = HIGH;  // Power LED but without PWM effect
    powerLED = 0;    // elapsedMillis
  }
  if (powerLED > 150) {
    LED_var = LOW;
    downLED = 0;  // elapsedSeconds
  }
  if (downLED > 2 && LED_var) {  // if Power Led is on (cleaned up from pwm) is on since at least 2sec (elapsedSeconds)
    machine_ready = HIGH;
  } else {
    machine_ready = LOW;
  }
}
//-------------------------------------------//
void updateVirtualLed(int LED_state_now) {
  static int last_LED_state = -1;
  // keine Display-Aktion wenn LED unverändert
  if (LED_state_now == last_LED_state) {
    return;
  }
  last_LED_state = LED_state_now;
  if (LED_state_now == HIGH) {
    tft.fillCircle(VLED_X, VLED_Y, VLED_R, COLOR_ON);
  } else {
    tft.fillCircle(VLED_X, VLED_Y, VLED_R, COLOR_OFF_FILL);
    tft.drawCircle(VLED_X, VLED_Y, VLED_R, COLOR_OFF_RING);
    // Optional: zweiten Ring für stärkere Kontur
    // tft.drawCircle(VLED_X, VLED_Y, VLED_R - 1, COLOR_OFF_RING);
  }
}
//-------------------------------------------//
void drawRedTreeSymmetric() {
  uint16_t treeColor  = ST77XX_RED;
  uint16_t trunkColor = ST77XX_RED;
  uint16_t bgColor    = ST77XX_BLACK;

  int w = tft.width();    // 160
  int h = tft.height();   // 128

  //tft.fillScreen(bgColor);

  int centerX   = w / 2;   // 80
  int topMargin = 50;      // 50 px Abstand oben

  // Baumparameter (schmalere Version)
  const int levels         = 4;
  const int heightPerLevel = 16;
  const int baseWidth      = 90;   // vorher 120 -> jetzt schmaler

  // ------------------------------------
  // 1) Dreiecke zeichnen (Schmal-Version)
  // ------------------------------------
  for (int i = 0; i < levels; i++) {
    int yTop    = topMargin + i * heightPerLevel;
    int yBottom = yTop + heightPerLevel;

    // schmalerer Faktor
    float widthFactor = 0.28f + (float)i / (levels + 1);
    int width = (int)(baseWidth * widthFactor);

    int xLeft  = centerX - width / 2;
    int xRight = centerX + width / 2;

    tft.fillTriangle(
      xLeft,   yBottom,
      centerX, yTop,
      xRight,  yBottom,
      treeColor
    );
  }

  // Unterkante der unteren Stufe
  int baseY = topMargin + levels * heightPerLevel;

  // -------------------------------
  // 2) Stamm (leicht schmaler)
  // -------------------------------
  const int trunkWidth  = 16;     // vorher 18
  const int trunkHeight = 18;

  int trunkX = centerX - trunkWidth / 2;
  int trunkY = baseY + 2;

  tft.fillRect(trunkX, trunkY, trunkWidth, trunkHeight, trunkColor);

  // -------------------------------
  // 3) Stern (integriert)
  // -------------------------------
  int   starCx = centerX;
  int   starCy = topMargin - 12;
  float outerR = 8.0f;
  float innerR = 4.0f;

  for (int k = 0; k < 10; k++) {
    int k2 = (k + 1) % 10;

    float angle1 = PI / 2.0f + k  * (PI / 5.0f);
    float angle2 = PI / 2.0f + k2 * (PI / 5.0f);

    float r1 = (k  % 2 == 0) ? outerR : innerR;
    float r2 = (k2 % 2 == 0) ? outerR : innerR;

    int x1 = starCx + (int)roundf(r1 * cosf(angle1));
    int y1 = starCy - (int)roundf(r1 * sinf(angle1));
    int x2 = starCx + (int)roundf(r2 * cosf(angle2));
    int y2 = starCy - (int)roundf(r2 * sinf(angle2));

    tft.drawLine(x1, y1, x2, y2, treeColor);
  }
}

/*--Hilfsfunktion zum Ausführen von SQL--*/
int db_exec(const char *sql) 
{
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK) {
        Serial.printf("SQL Fehler: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rc;
}
/*---------------------------------------*/

/*--Nutzer suchen oder anlegen in DB--*/
void sucheNutzer(uint64_t gesuchteID)
{
  char *zErrMsg  = 0;
  sqlite3_stmt *res;  //res dient hier als Speicher

  // Hier der Part vom anlegen
  String insertSql = "INSERT OR IGNORE INTO kaffee_nutzer (id, saldo, anzahl_kaffees) VALUES (" + String(gesuchteID) + " , 0.0, 0);"; // Befehl zum Einfügen oder Ignorieren falls vorhanden

  int rc = sqlite3_exec(db, insertSql.c_str(), NULL, NULL, &zErrMsg);
  if (rc != SQLITE_OK)
  {
    Serial.printf("Fehler bei SQL-Funktion Insert: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return;
  }

  //  Hier der Part vom suchen
  const char *sql = "SELECT saldo, anzahl_kaffees FROM kaffee_nutzer WHERE id = ?;";    //  Direkt alle Daten vom Nutzer auslesen
  if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) == SQLITE_OK)   //  db = Datenbankverbindung ; sql = SELECT Anweisung ; &res = Unser Speicher ; -1 = Lesen bis \0 also ende
  {
    sqlite3_bind_int64(res, 1, gesuchteID); 
    if (sqlite3_step(res) == SQLITE_ROW)  //  Einmal durch die DB durch gehen bis die ID gefunden wurde
    {
      saldo = sqlite3_column_double(res, 0);
      counter = sqlite3_column_int(res, 1);
    }
    sqlite3_finalize(res);  // res wieder freigeben fürs nächste mal
  }
}
/*---------------------------------------*/

/*--Entlastung der Loop durch FreeRTOS--*/
void entlastung (void *pvParameters)
{
  while (1)
  {
    vTaskDelay(20 / portTICK_PERIOD_MS);  //  Durchlaufzeit

    readRFID();
    updateButton();
    machineReady();
    //  Debugging ohne Kaffeemaschiene sodass man Kaffee ziehen kann
    // LED_var = 1;
    // machine_ready = 1;
  }
}
/*---------------------------------------*/

/*--Kaffee kaufen und Zähler--*/
void buchung(uint64_t gesuchteID, float neuesSaldo, int neuerCounter)
{
  sqlite3_stmt *res;
  const char *sql = "UPDATE kaffee_nutzer SET saldo = ?, anzahl_kaffees = ? WHERE id = ?;";

  if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) == SQLITE_OK)
  {
    sqlite3_bind_double(res, 1, neuesSaldo);  /**************/
    sqlite3_bind_int(res, 2, neuerCounter);   /* Hier die Fragezeichen von oben füllen*/
    sqlite3_bind_int64(res, 3, gesuchteID);   /**************/

    if (sqlite3_step(res) == SQLITE_DONE)
    {
      Serial.println("Erfolgreich alles aktualisiert...");
    }
    sqlite3_finalize(res);
  }
}
/*---------------------------------------*/

/*--Aufladen eigentlich genau wie buchung nur ohne Counter (zu spät um buchung zu optimieren -.-)--*/
void aufladen(uint64_t gesuchteID, float neuesSaldo)
{
  sqlite3_stmt *res;
  const char *sql = "UPDATE kaffee_nutzer SET saldo = ? WHERE id = ?;";

  if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) == SQLITE_OK)
  {
    sqlite3_bind_double(res, 1, neuesSaldo);
    sqlite3_bind_int64(res, 2, gesuchteID);

    if (sqlite3_step(res) == SQLITE_DONE)
    {
      Serial.println("Erfolgreich augeladen und Salod aktualisiert...");
    }
    sqlite3_finalize(res);
  }
}
/*---------------------------------------*/
