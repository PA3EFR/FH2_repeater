After installation I noticed that the buttons and their associated functions were mixed up.

When I pressed 1, the radio went up in frequency; when I pressed 2 it transmitted text 4.

The solution is in the binary code for the MUX. 
I expected 0001 for number 1, but it gave me up, which is 1000
I expected 0010 for number 2, but it gave me 4, which is 0100

The quick software fix is to adjust this part of the INO file (line 19 and on):
// ---------------------------------------------------------------------------
// PIN DEFINITIES - Nano specifiek
// ---------------------------------------------------------------------------
#define PIN_S0           2   // Nano D2: MUX S0
#define PIN_S1           3   // Nano D3: MUX S1
#define PIN_S2           4   // Nano D4: MUX S2
#define PIN_S3           5   // Nano D5: MUX S3

Change it to:
// ---------------------------------------------------------------------------
// PIN DEFINITIES - Nano specifiek
// ---------------------------------------------------------------------------
#define PIN_S0           5   // Nano D2: MUX S0
#define PIN_S1           4   // Nano D3: MUX S1
#define PIN_S2           3   // Nano D4: MUX S2
#define PIN_S3           2   // Nano D5: MUX S3
