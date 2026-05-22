// ============================================================
// FH-2 CQ-machine controller — Arduino NANO versie 6
// Met intervaltijd weergave in monitor
// Repeat met interval beperkt tot knoppen 1, 2, 3, 4, 5
// ============================================================

// ---------------------------------------------------------------------------
// PULS DEFINITIES
// ---------------------------------------------------------------------------
#define PULS_FH2     "FH-2"
#define PULS_AUTO    "AUTO"

// ---------------------------------------------------------------------------
// STATUS DEFINITIES
// ---------------------------------------------------------------------------
#define STATUS_GESTOPT "GESTOPT"
#define STATUS_ACTIEF  "ACTIEF"

// ---------------------------------------------------------------------------
// PIN DEFINITIES - Nano specifiek
// ---------------------------------------------------------------------------
#define PIN_S0           2   // Nano D2: MUX S0
#define PIN_S1           3   // Nano D3: MUX S1
#define PIN_S2           4   // Nano D4: MUX S2
#define PIN_S3           5   // Nano D5: MUX S3
#define PIN_REPEAT_EN    6   // Nano D7: Repeat enable (LOW=uit, HIGH=aan)
#define PIN_PULSE_LED    7   // Nano D9: Puls indicator LED
#define PIN_STATUS_LED   8  // Nano D10: Status LED (GROEN)
#define PIN_POTMETER     A1  // Nano A1: Potmeter interval
#define PIN_ADC_IN       A0  // Nano A0: ADC ingang FH-2

// ---------------------------------------------------------------------------
// CONSTANTEN
// ---------------------------------------------------------------------------
#define ADC_MAX         1023
#define VREF            5.0
#define POT_MAX         1023
#define MIN_INTERVAL    10       // minimale interval in seconden
#define MAX_INTERVAL    60      // maximale interval in seconden
#define MUX_TIMEOUT_MS  2000    // 2 seconden timeout
#define PULSE_DURATION  250     // 500ms pulsduur
#define DEBOUNCE_MS     50      // 50ms debounce
#define TOLERANTIE      0.15    // Spannings tolerantie voor knopdetectie

// ---------------------------------------------------------------------------
// GEMETEN SPANNINGEN EN KNOPEN
// ---------------------------------------------------------------------------
#define NUM_BUTTONS 13

const float BUTTON_VOLTAGES[NUM_BUTTONS] = {
    2.53,   // 0: Geen knop (rust) - hoogste spanning
    0.38,   // 1: 1.text - laagste spanning
    0.51,   // 2: 2.text
    0.667,  // 3: 3.text
    0.817,  // 4: 4.text
    1.001,  // 5: 5.text
    1.165,  // 6: MEM
    1.274,  // 7: Links (LEFT)
    1.453,  // 8: Up (UP)
    1.628,  // 9: Rechts (RIGHT)
    1.782,  // 10: Down (DOWN)
    1.922,  // 11: P/B
    2.09    // 12: DEC
};

const char* BUTTON_NAMES[NUM_BUTTONS] = {
    "-",       // 0: Geen knop
    "1",       // 1: 1.text
    "2",       // 2: 2.text
    "3",       // 3: 3.text
    "4",       // 4: 4.text
    "5",       // 5: 5.text
    "MEM",     // 6: MEM
    "Links",   // 7: LEFT
    "Up",      // 8: UP
    "Rechts",  // 9: RIGHT
    "Down",    // 10: DOWN
    "P/B",     // 11: P/B
    "DEC"      // 12: DEC
};

const int MUX_CHANNELS[NUM_BUTTONS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

// ---------------------------------------------------------------------------
// VARIABELEN
// ---------------------------------------------------------------------------
int previousButton = -1;
int activeButton = -1;
unsigned long lastButtonPress = 0;
bool intervalActive = false;
unsigned long lastIntervalPulse = 0;
int intervalSeconds = 5;
bool repeatEnabled = true;
unsigned long lastPulseTime = 0;
bool pulseLedState = false;
int lastPotValue = -1;

// Vorige waarden voor change detection
int prev_intervalSeconds = -1;
bool prev_intervalActive = false;
bool prev_repeatEnabled = false;
int prev_detectedButton = -1;

// ---------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(100);
    }

    // Configureer pinnen
    pinMode(PIN_S0, OUTPUT);
    pinMode(PIN_S1, OUTPUT);
    pinMode(PIN_S2, OUTPUT);
    pinMode(PIN_S3, OUTPUT);
    pinMode(PIN_MUX_EN, OUTPUT);
    pinMode(PIN_REPEAT_EN, INPUT_PULLUP);
    pinMode(PIN_PULSE_LED, OUTPUT);
    pinMode(PIN_STATUS_LED, OUTPUT);

    // MUX uit bij opstart
    digitalWrite(PIN_MUX_EN, HIGH);
    setMuxChannel(0);

    // LED's uit
    digitalWrite(PIN_PULSE_LED, LOW);
    digitalWrite(PIN_STATUS_LED, LOW);

    // Lees initiële interval
    intervalSeconds = readInterval();
    lastPotValue = intervalSeconds;
    prev_intervalSeconds = intervalSeconds;

    // Toon info
    printHeader();
    printCurrentSettings();

    Serial.println(F("=========================================="));
    Serial.println(F("  Systeem gereed. Wacht op FH-2 knop..."));
    Serial.println(F("==========================================\n"));

    delay(500);
}

// ---------------------------------------------------------------------------
// KNOP DETECTIE
// ---------------------------------------------------------------------------
bool canRepeatWithInterval(int buttonIndex) {
    // Alleen knoppen 1, 2, 3, 4, 5 kunnen de interval-cyclus gebruiken
    return (buttonIndex >= 1 && buttonIndex <= 5);
}

int detectButtonByVoltage(int adcValue) {
    float measuredVoltage = (adcValue * VREF) / ADC_MAX;

    int bestMatch = 0;
    float minDiff = 999.0;

    for (int i = 0; i < NUM_BUTTONS; i++) {
        float diff = abs(BUTTON_VOLTAGES[i] - measuredVoltage);
        if (diff < minDiff) {
            minDiff = diff;
            bestMatch = i;
        }
    }

    if (minDiff < TOLERANTIE) {
        return bestMatch;
    }

    return 0;
}

// ---------------------------------------------------------------------------
// HEADER WEERGAVE
// ---------------------------------------------------------------------------
void printHeader() {
    Serial.println(F("=========================================="));
    Serial.println(F("  FH-2 CQ-machine - Arduino NANO v6"));
    Serial.println(F("  Met intervaltijd weergave"));
    Serial.println(F("=========================================="));
    Serial.println();
    Serial.println(F("NANO PIN LAYOUT:"));
    Serial.println(F("------------------------------------------"));
    Serial.println(F("  A0 = ADC FH-2 input"));
    Serial.println(F("  A1 = Potmeter interval"));
    Serial.println(F("  D5-D2 = MUX address (S0-S3)"));
    Serial.println(F("  D6 = MUX ~EN"));
    Serial.println(F("  D7 = Repeat schakelaar (GND=uit)"));
    Serial.println(F("  D9 = Puls LED"));
    Serial.println(F("  D10 = Status LED (GROEN)"));
    Serial.println();
}

// ---------------------------------------------------------------------------
// HUIDIGE INSTELLINGEN WEERGAVE
// ---------------------------------------------------------------------------
void printCurrentSettings() {
    Serial.println(F("HUIDIGE INSTELLINGEN:"));
    Serial.println(F("------------------------------------------"));
    Serial.print(F("  Interval: "));
    Serial.print(intervalSeconds);
    Serial.println(F(" sec"));
    Serial.print(F("  Repeat: "));
    Serial.println(repeatEnabled ? F("AAN") : F("UIT"));
    Serial.println();
}

// ---------------------------------------------------------------------------
// MUX KANAAL INSTELLEN
// ---------------------------------------------------------------------------
void setMuxChannel(int channel) {
    digitalWrite(PIN_S3, (channel & 1) ? HIGH : LOW);
    digitalWrite(PIN_S2, (channel & 2) ? HIGH : LOW);
    digitalWrite(PIN_S1, (channel & 4) ? HIGH : LOW);
    digitalWrite(PIN_S0, (channel & 8) ? HIGH : LOW);
}

// ---------------------------------------------------------------------------
// PULS VERZENDEN
// ---------------------------------------------------------------------------
void sendPulse(int muxChannel, const char* pulseType) {
    digitalWrite(PIN_MUX_EN, LOW);
    setMuxChannel(muxChannel);

    delay(PULSE_DURATION);

    digitalWrite(PIN_MUX_EN, HIGH);
    setMuxChannel(0);

    // Pulse LED activeren
    digitalWrite(PIN_PULSE_LED, HIGH);
    lastPulseTime = millis();
    pulseLedState = true;

    Serial.println();
    Serial.print(F("["));
    Serial.print(millis() / 1000);
    Serial.print(F("s] PULS: "));
    Serial.print(BUTTON_NAMES[activeButton]);
    Serial.print(F(" | Type: "));
    Serial.print(pulseType);
    Serial.print(F(" | Interval: "));
    Serial.print(intervalSeconds);
    Serial.print(F("s | MUX: "));
    Serial.println(muxChannel);
}

// ---------------------------------------------------------------------------
// INTERVAL LEZEN
// ---------------------------------------------------------------------------
int readInterval() {
    int potValue = analogRead(PIN_POTMETER);
    int interval = map(potValue, 0, POT_MAX, MIN_INTERVAL, MAX_INTERVAL);
    return interval;
}

// ---------------------------------------------------------------------------
// STATUS WIJZIGING REPORTEN
// ---------------------------------------------------------------------------
void reportStatusChange(const char* what, const char* oldValue, const char* newValue) {
    Serial.print(F("["));
    Serial.print(millis() / 1000);
    Serial.print(F("s] STATUS: "));
    Serial.print(what);
    Serial.print(F(" veranderd: "));
    Serial.print(oldValue);
    Serial.print(F(" -> "));
    Serial.println(newValue);
}

// ---------------------------------------------------------------------------
// KNOP WIJZIGING REPORTEN
// ---------------------------------------------------------------------------
void reportButtonChange(int oldButton, int newButton, int adcValue) {
    Serial.println();
    Serial.print(F("["));
    Serial.print(millis() / 1000);
    Serial.print(F("s] KNOP: "));
    Serial.print(BUTTON_NAMES[oldButton]);
    Serial.print(F(" -> "));
    Serial.print(BUTTON_NAMES[newButton]);
    Serial.print(F(" (ADC="));
    Serial.print(adcValue);
    Serial.print(F(", "));
    Serial.print((adcValue * VREF) / ADC_MAX, 2);
    Serial.print(F("V) | Interval: "));
    Serial.print(intervalSeconds);
    Serial.println(F("s"));
}

// ---------------------------------------------------------------------------
// INTERVAL CYCLUS START REPORTEN
// ---------------------------------------------------------------------------
void reportCycleStart() {
    Serial.println();
    Serial.println(F("=========================================="));
    Serial.println(F(">>> INTERVAL CYCLUS GESTART"));
    Serial.print(F(">>> Knop: "));
    Serial.print(BUTTON_NAMES[activeButton]);
    Serial.print(F(" | Interval: "));
    Serial.print(intervalSeconds);
    Serial.println(F(" sec"));
    Serial.print(F(">>> Repeat: "));
    Serial.println(repeatEnabled ? F("AAN") : F("UIT"));
    Serial.println(F(">>> Druk opnieuw op dezelfde knop om te stoppen"));
    Serial.println(F("=========================================="));
    Serial.println();
}

// ---------------------------------------------------------------------------
// INTERVAL CYCLUS STOP REPORTEN
// ---------------------------------------------------------------------------
void reportCycleStop() {
    Serial.println();
    Serial.println(F("=========================================="));
    Serial.println(F(">>> INTERVAL CYCLUS GESTOPT"));
    Serial.println(F("=========================================="));
    Serial.println();
}

// ---------------------------------------------------------------------------
// INTERVAL WIJZIGING REPORTEN
// ---------------------------------------------------------------------------
void reportIntervalChange(int oldInterval, int newInterval) {
    Serial.print(F("["));
    Serial.print(millis() / 1000);
    Serial.print(F("s] INFO: Interval gewijzigd: "));
    Serial.print(oldInterval);
    Serial.print(F("s -> "));
    Serial.print(newInterval);
    Serial.println(F("s"));
}

// ---------------------------------------------------------------------------
// HOOFDLUS
// ---------------------------------------------------------------------------
void loop() {
    unsigned long now = millis();
    int adcValue = analogRead(PIN_ADC_IN);
    int detectedButton = detectButtonByVoltage(adcValue);
    repeatEnabled = (digitalRead(PIN_REPEAT_EN) == HIGH);

    // Pulse LED timer
    if (pulseLedState && (now - lastPulseTime >= 1000)) {
        digitalWrite(PIN_PULSE_LED, LOW);
        pulseLedState = false;
    }

    // Interval bijwerken
    int currentInterval = readInterval();
    if (currentInterval != lastPotValue) {
        lastPotValue = currentInterval;
        intervalSeconds = currentInterval;
    }

    // Status LED bijwerken
    digitalWrite(PIN_STATUS_LED, intervalActive ? HIGH : LOW);

    // ========================================
    // CHANGE DETECTION EN RAPPORTAGE
    // ========================================

    // Repeat schakelaar wijziging
    if (repeatEnabled != prev_repeatEnabled) {
        reportStatusChange("Repeat", 
            prev_repeatEnabled ? "AAN" : "UIT", 
            repeatEnabled ? "AAN" : "UIT");
        prev_repeatEnabled = repeatEnabled;
    }

    // Interval wijziging rapporteren
    if (intervalSeconds != prev_intervalSeconds) {
        reportIntervalChange(prev_intervalSeconds, intervalSeconds);
        prev_intervalSeconds = intervalSeconds;
    }

    // Status wijziging
    if (intervalActive != prev_intervalActive) {
        if (intervalActive) {
            reportCycleStart();
        } else {
            reportCycleStop();
        }
        prev_intervalActive = intervalActive;
    }

    // Knop wijziging
    if (detectedButton != prev_detectedButton) {
        if (detectedButton > 0) {
            reportButtonChange(prev_detectedButton, detectedButton, adcValue);
        }
        prev_detectedButton = detectedButton;
    }

    // ========================================
    // KNOP VERWERKING
    // ========================================

    if (detectedButton > 0 && detectedButton != previousButton) {
        delay(DEBOUNCE_MS);

        int checkAdc = analogRead(PIN_ADC_IN);
        int checkButton = detectButtonByVoltage(checkAdc);

        if (checkButton == detectedButton) {
            activeButton = detectedButton;
            lastButtonPress = now;
            prev_intervalSeconds = intervalSeconds;

            // Stuur puls
            int muxCh = MUX_CHANNELS[activeButton];
            sendPulse(muxCh, PULS_FH2);

            // Intervalcyclus starten of stoppen (alleen voor knoppen 1-5)
            if (canRepeatWithInterval(activeButton)) {
                if (!intervalActive) {
                    intervalActive = true;
                    lastIntervalPulse = now;
                    prev_intervalActive = intervalActive;
                } else {
                    intervalActive = false;
                    digitalWrite(PIN_STATUS_LED, LOW);
                    prev_intervalActive = intervalActive;
                }
            } else {
                // Overige knoppen stoppen een actieve interval-cyclus
                if (intervalActive) {
                    intervalActive = false;
                    digitalWrite(PIN_STATUS_LED, LOW);
                    prev_intervalActive = intervalActive;
                }
            }

            previousButton = detectedButton;
        }
    }

    if (detectedButton == 0) {
        previousButton = -1;
    }

    // ========================================
    // INTERVAL PULS
    // ========================================

    if (intervalActive && repeatEnabled) {
        if (now - lastIntervalPulse >= (intervalSeconds * 1000UL)) {
            sendPulse(MUX_CHANNELS[activeButton], PULS_AUTO);
            lastIntervalPulse = now;
        }
    }

    // MUX timeout
    if (activeButton > 0 && (now - lastButtonPress >= MUX_TIMEOUT_MS)) {
        setMuxChannel(0);
        digitalWrite(PIN_MUX_EN, HIGH);
    }
}