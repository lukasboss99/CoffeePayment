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
  if (timerRFID > 500) {
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
  //Serial.println(scaledValue);
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

