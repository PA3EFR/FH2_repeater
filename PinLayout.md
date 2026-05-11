## **Nano Pin Layout**



NANO PINOUT - FH-2 CQ-MACHINE

========================



Voeding:

&#x20; VCC  → 5V (van USB of externe voeding)

&#x20; GND  → GND (massa)



Communicatie:

&#x20; (USB) → Serieel (9600 baud)



Analoge pinnen:

&#x20; A0   → FH-2 ADC input (weerstandsladder)

&#x20; A1   → Potmeter interval (10kΩ naar 5V en GND)



Digitale pinnen:

&#x20; D2   → MUX S0 (address bit 0)

&#x20; D3   → MUX S1 (address bit 1)

&#x20; D4   → MUX S2 (address bit 2)

&#x20; D5   → MUX S3 (address bit 3)

&#x20; D6   → MUX \~EN (enable, actief laag)

&#x20; D7   → Repeat schakelaar (naar GND = uit)

&#x20; D9   → Puls LED (ROOD, via 220Ω naar GND)

&#x20; D10  → Status LED (GROEN, via 220Ω naar GND)



## 

## **Aansluitschema Nano**



Arduino Nano          74HC4067 MUX

\---------            -------------

&#x20; D2 (S0)    ────    S0

&#x20; D3 (S1)    ────    S1

&#x20; D4 (S2)    ────    S2

&#x20; D5 (S3)    ────    S3

&#x20; D6 (\~EN)   ────    \~EN

&#x20; D7         ────    Repeat schakelaar (naar GND)

&#x20; D9         ────    \[RODE LED] ── 220Ω ── GND

&#x20; D10        ────    \[GROENE LED] ── 220Ω ── GND

&#x20; A0         ────    FH-2 ADC signaal

&#x20; A1         ────    Potmeter signaal

&#x20; 5V         ────    VCC (MUX en potmeter)

&#x20; GND        ────    GND (alles)

