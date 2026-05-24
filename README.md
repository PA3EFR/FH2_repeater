**FH-2 Repeater - Functional and Technical Descriptions**

**Functional Description**
The FH-2 Repeater is an Arduino NANO-based system that provides automated control for the Yaesu FH-2 Voice keyer, enabling users to send pre-programmed (CQ) messages at configurable intervals. This controller bridges the gap between traditional manual keying and modern automation, making it particularly useful for contest stations and field operations where consistent, timed CQ transmissions are essential.

The system operates by reading the FH-2's button matrix through an analog voltage sensing mechanism, where each button produces a distinct voltage level through a resistor network. When a button is pressed, the controller activates the corresponding channel on a 16-channel analog multiplexer (74HC4067) to send the appropriate control pulse to the FH-2 unit. The core functionality centers on the interval repeat feature, which allows buttons 1 through 5 to automatically repeat their transmission at user-defined intervals ranging from 10 to 60 seconds. A front-panel potentiometer provides real-time adjustment of the interval timing, while a toggle switch enables or disables the repeat functionality entirely.

The user interface consists of minimal but effective visual indicators: a pulse LED flashes briefly when a transmission occurs, and a status LED illuminates when an interval cycle is actively running. All system information, including button changes, interval modifications, and cycle status, is streamed to the serial monitor at 9600 baud, allowing operators to verify correct operation and troubleshoot any issues. The interval cycle initiates when button 1, 2, 3, 4, or 5 is pressed and continues automatically until the same button is pressed again or the repeat function is disabled via the toggle switch. Pressing any other button immediately terminates an active interval cycle, providing a quick emergency stop mechanism.

The controller's design philosophy prioritizes reliability and simplicity, using only through-hole components for easy construction and maintenance. The debounce algorithm ensures that only genuine button presses are registered, preventing false triggers from mechanical switch bounce. The multiplexer timeout feature ensures that no channel remains selected indefinitely, protecting both the multiplexer and the connected equipment from potential damage due to software or hardware faults.

**Technical Description**
The FH-2 Repeater employs an Arduino NANO (ATmega328P) as its processing core, utilizing the microcontroller's analog-to-digital converter for button matrix scanning and its digital I/O pins for multiplexer control and LED indication. The pin configuration has been carefully chosen to optimize the limited I/O available on the NANO form factor while maintaining logical organization of the various subsystems.

Pin Assignment Summary:

Function	Pin	Description
ADC Input	A0 (Pin 14)	FH-2 button voltage measurement
Interval Control	A1 (Pin 15)	Potentiometer wiper for interval adjustment
MUX S0-S3	D2-D5 (Pins 2-5)	4-bit multiplexer channel selection
MUX Enable	D6 (Pin 6)	Active-low enable for 74HC4067
Repeat Switch	D7 (Pin 7)	Pullup input for repeat on/off toggle
Pulse LED	D9 (Pin 9)	Visual pulse confirmation indicator
Status LED	D10 (Pin 10)	Interval cycle active indicator
The button detection system operates by measuring the voltage present on pin A0, which is connected to the FH-2's button matrix through a voltage divider network. Each button produces a characteristic voltage between 0.38V and 2.53V, with the controller comparing the measured voltage against a lookup table containing all 13 button voltage thresholds. The tolerance window of ±0.15V ensures reliable detection even when minor voltage variations occur due to power supply fluctuations or temperature drift. The best-match algorithm selects the button whose voltage profile most closely matches the measured value, providing robust detection across the entire button matrix.

The multiplexer interface uses four address lines (S0-S3) to select one of sixteen possible channels, with the enable line (active-low) controlling when the multiplexer output is connected to the FH-2 input. When a pulse is triggered, the multiplexer is enabled, the appropriate channel is selected, and after a 250ms dwell time, the multiplexer is disabled and reset to channel 0. A 2-second timeout ensures the multiplexer returns to a safe state if button press detection fails to complete normally.

The interval system implements a simple state machine with three primary states: idle, active-cycling, and stopped. When the user presses buttons 1-5 while repeat is enabled, the system enters the active-cycling state and records the timestamp of the initial pulse. Every subsequent interval period (configurable 10-60 seconds via potentiometer), the same button pulse is automatically resent until the user presses the same button again to stop, presses a different button to override, or disables the repeat switch. The potentiometer is sampled continuously, and any change in the interval setting is immediately reflected in the cycle timing, allowing real-time adjustment without interrupting the current cycle.

Software architecture follows a modular structure with distinct functions for button detection, multiplexer control, pulse generation, and serial reporting. The main loop implements a cooperative multitasking approach, checking all inputs and updating outputs in sequence at approximately 100Hz. The debounce logic requires two consecutive identical button readings separated by 50ms before accepting a button press as valid, effectively filtering out mechanical bounce artifacts. Serial output uses formatted strings with timestamp information (seconds since startup) to facilitate post-operation analysis and debugging.

The circuit requires a regulated 5V supply capable of delivering approximately 100mA for the Arduino, multiplexer, and LED indicators. Power can be sourced from the USB connection during development or from an external 5V regulator for permanent installation. The FH-2 connection uses a standard 3.5mm stereo jack, with the multiplexer output directly driving the FH-2's button matrix inputs according to the channel mapping defined in the source code.
