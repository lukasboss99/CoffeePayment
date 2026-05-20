//--------------------------STATE 1----------------------------//
void state1() {  // Idle, bitte Keytag auflegen
  if (machine.executeOnce) {
    current_state = 1;
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 40);
    tft.print("BITTE");

    tft.setCursor(20, 60);
    tft.print("KEYTAG");

    tft.setCursor(20, 80);
    tft.print("AUFLEGEN");
  }
}
bool transitionS1S2() {
  if (cardPresent) {
    return true;
  }
  return false;
}
//--------------------------STATE 2----------------------------//
void state2() {  // Auswahl Kaffee bzw. Aufladen
  if (machine.executeOnce) {
    current_state = 2;
    foundUID = false;
    sucheNutzer(uidDec);
    // Header: Nutzer-Nr und Saldo
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
    // Menü
    tft.setTextSize(3);
    tft.setCursor(30, 60);
    tft.print("KAFFEE");
    tft.setCursor(30, 95);
    tft.print("LADEN");
    // Neuen Pfeil zeichnen
    if (selectionMenu == 0) drawArrow(30, 60);
    else if (selectionMenu == 1) drawArrow(30, 95);
    // LED initial zeichnen
    if (LED_var == HIGH) {
      tft.fillCircle(VLED_X, VLED_Y, VLED_R, COLOR_ON);
    } else {
      tft.fillCircle(VLED_X, VLED_Y, VLED_R, COLOR_OFF_FILL);
      tft.drawCircle(VLED_X, VLED_Y, VLED_R, COLOR_OFF_RING);
    }
  }
  if (rawValue % 2 == 0) {
    selectionMenu = 0;
  } else {
    selectionMenu = 1;
  }
  updateArrow(selectionMenu);
  updateVirtualLed(LED_var);
}
bool transitionS2S3() {
  if (selectionMenu == 0 && ok_button) {
    bezug = 0;            // timer für die Dauer des "state: bezug" starten
    if (machine_ready) {  // Bezug nur zulassen, wenn Maschine auch angeschalten ist (Optokoppler an LED ON/OFF Knopf)
      return true;
    }
  }
  return false;
}
bool transitionS2S4() {
  if (selectionMenu == 1 && ok_button) {
    rotaryEncoder.setEncoderValue(0);
    stateJump = 0;  // Verhindert, dass wir mit dem aktuellen "OK" den nächsten State direkt überspringen
    return true;
  }
  return false;
}
bool transitionS2S1() {
  if (restartRequested) {
    return true;
  }
  return false;
}
//--------------------------STATE 3----------------------------//
void state3() {  // Kaffeebezug
  if (machine.executeOnce) {
    current_state = 3;
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setCursor(62, 40);
    tft.print("...");
    t_relais = 0;
    digitalWrite(PIN_RELAIS, HIGH);
    saldo = saldo - preis;
    timerRFID = 0;
    counter = counter + 1;
    timerRFID = 0;
    timerRFID = 0;
    //  Hier die DB aktualisieren
    buchung(uidDec, saldo, counter);

    tft.setCursor(30, 80);
    tft.print("-");
    tft.setCursor(45, 80);
    tft.print(preis);
    tft.setCursor(100, 80);
    tft.print("EUR");

    tft.setTextSize(1);
    tft.setCursor(35, 110);
    tft.print("COFFEE NO.:");
    tft.setCursor(105, 110);
    tft.print(counter);

    /*HOLIDAY UPDATE
    tft.setCursor(30, 10);
    tft.print("-");
    tft.setCursor(45, 10);
    tft.print(preis);
    tft.setCursor(100, 10);
    tft.print("EUR");
    drawRedTreeSymmetric();
    */
  }
  if (t_relais > 500) {
    digitalWrite(PIN_RELAIS, LOW);
  }
}
bool transitionS3S2() {
  if (bezug > 40000) {
    digitalWrite(PIN_RELAIS, LOW);
    return true;
  }
  return false;
}
bool transitionS3S1() {
  if (restartRequested && bezug > 10000) { // Sicherstellen, dass die Balance auch geschrieben wird
    delay(250);
    digitalWrite(PIN_RELAIS, LOW);
    return true;
  }
  return false;
}
//--------------------------STATE 4----------------------------//
void state4() {  // Aufladen
  if (machine.executeOnce) {
    current_state = 4;
    rotaryEncoder.setBoundaries(0, 4, false);
    // Header: Nutzer-Nr und Saldo
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
  }
}
bool transitionS4S5() {
  if (ok_button && stateJump > 2000) {
    saldo = saldo + (float)scaledValue;
    ladebetrag = scaledValue;
    aufladen(uidDec, saldo);
    rotaryEncoder.setBoundaries(-1000, 1000, false);
    return true;
  }
  return false;
}
bool transitionS4S1() {
  if (restartRequested) {
    return true;
  }
  return false;
}

//--------------------------STATE 5----------------------------//
void state5() {  // Aufladen bestätigen
  if (machine.executeOnce) {
    aufladebestaetigung = 0;
    current_state = 5;
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);
    // Zeile 1: ladebetrag + " EUR AUFGELADEN"
    tft.setCursor(16, 10);  // Zentriert für ca. 13 Zeichen
    tft.print(ladebetrag, 2);
    tft.print(" EUR AUFGELADEN");
    tft.setTextSize(2);
    // Zeile 2: "SALDO NEU:"
    tft.setCursor(16, 40);  // Zentriert für 10 Zeichen
    tft.print("SALDO NEU:");

    // Zeile 3: saldo-Wert
    tft.setCursor(16, 70);  // Zentriert für ca. 9 Zeichen
    tft.print(saldo, 2);
    tft.print(" EUR");
  }
}
bool transitionS5S2() {
  if (aufladebestaetigung > 2000) {
    return true;
  }
  return false;
}
bool transitionS5S1() {
  if (restartRequested) {
    return true;
  }
  return false;
}
