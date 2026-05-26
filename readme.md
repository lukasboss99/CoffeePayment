## PINOUT
| Modul       | Signal       | GPIO    |
|-------------|--------------|---------|
| **RFID**    | CS ("SDA")   | GPIO 4  |
| **DISPLAY** | CS           | GPIO 15 |
|             | RS/DC        | GPIO 16 |
|             | RST          | GPIO 7  |
| **SD-Karte**| CS           | GPIO 47 |
| **ENCODER** | SW           | GPIO 42 |
|             | DT ("B")     | GPIO 40 |
|             | CLK ("A")    | GPIO 41 |
| **RELAIS**  | GPIO         | 48      |

## SPI-Bus-Zuweisungen

### SPI für SD-Karte

| Signal | GPIO |
|--------|------|
| SCK    | 19   |
| MISO   | 20   |
| MOSI   | 21   |

### SPI für TFT & RFID

| Signal     | GPIO |
|------------|------|
| SCK        | 36   |
| MISO       | 37   |
| MOSI (SDA) | 35   |

## Aufbau der Datenbank

SQLite-Datei: `kaffee.db` (auf SD)
Programm zum Auswerten der Datenbank auf dem PC: DB Browser for SQLite

Tabelle: `kaffee_nutzer`

| id (INTEGER, PK) | nutzer_nummer (INTEGER) | name (TEXT) | saldo (REAL) | anzahl_kaffees (INTEGER) |
|---|---|---|---|---|
| 287454020 | 999 | Verstappen | 20.50 | 12 |

Hinweis: Neue IDs werden automatisch mit `saldo = 0.0` und `anzahl_kaffees = 0` angelegt.
         Name wird noch nicht gesetzt ist aber schon vorhanden für eventuelle Updates!

## Aufbau State Machine

```mermaid
stateDiagram-v2
    [*] --> S1
    S1: WARTEN AUF SCAN (S1)
    S1 --> S2

    S2: MODUS AUSWAHL (S2)
    S2 --> S3
    S2 --> S4

    S3: BEZUG (S3)
    S3 --> S2

    S4: AUFLADEN (S4)
    S4 --> S5

    S5: AUFLADEN BESTÄTIGEN (S5)
    S5 --> S2
```

## Schaltung Optokoppler (STATE LED ON/OFF)
![Optokoppler-Schaltung](src/images/optokoppler.png)

