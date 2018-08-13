// wb6b Auto Tx Filter controller
// This is a minimal Sketch to automatically select
// the low pass filter needed for the frequency
// you are transmitting on.
//
// The frequency and selected filter may be selected
// on the LCD display and/or by LEDs connected to
// the filter relay enable pins.
//
// A hold enable mode switch can be added to let
// the selected filter remain selected during receive
// or, at the end of the transmission, to return to the
// default filter. The default filter should normally
// be the filter with the highest cutoff frequency.
//
// The frequency counter resolution is only set to
// the level needed to select a proper filter.
//
// The Arduino can count frequency up to
// about 6 or 8Mhz depending on CPU clock frequency.
// So the input to the Arduino should be preceded by
// a divide by eight prescaler to allow operation
// up to 30Mhz.
//
// It should be expected that users will add refinements
// to this Sketch as needed for their needs.
//
// The relay wiring should address switching in a default
// filter when no power is applied.
//
// The original use case for this auto filter is as
// a supplemental filter. In cases where this may be
// the only filter, sending the TX output to
// a dummy load until the proper filter can be
// determined and switched in may be worth considering.
//
// T. Stall, wb6b, 20180712
//
// Author: Tom Stall, WB6B <mtm<@>mountaintom<.>com>
// Copyright: 2018 Tom Stall
// Version: 0.1.1
// Date: 20180812
// License: GNU General Public License

/**
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <FreqCount.h>
#include <LiquidCrystal.h>

#define FILTER_ENB_PIN_01 A0
#define FILTER_ENB_PIN_02 A1
#define FILTER_ENB_PIN_03 A2
#define FILTER_ENB_PIN_04 A3
#define FILTER_ENB_PIN_05 13
#define FILTER_DEFAULT_ENB_PIN A0

#define FILTER_DEFAULT_NUMBER 1
#define PICK_LEVEL false

#define HOLD_MODE_PIN 12

#define LCD_COLUMNS 16
#define LCD_ROWS 2

//#define LCD_D4_PIN 5
#define LCD_D4_PIN 10
#define LCD_D5_PIN 6
#define LCD_D6_PIN 7
#define LCD_D7_PIN 8

#define LCD_RS_PIN 2
#define LCD_RW_PIN 3
#define LCD_E_PIN 4

// Reserve pins A4 and A5 for possible
// I2C usage.

// Note: There is no hysteresis on the filter selection
// logic. Choose frequencies outside the range of
// frequencies you would tune to within a band.
//
// These example filters may not be the best choices
// they are examples until best choices are determined.
// Filters that are closer to the uBITX cutoff
// frequencies may be better.
// The 160M filter is a teaser if everything
// can work with five filters.
// Cutout frequencies are in Khz
//
#define FILTER_CUTOUT_FREQ_01 30000
#define FILTER_CUTOUT_FREQ_02 16000
#define FILTER_CUTOUT_FREQ_03 9000
#define FILTER_CUTOUT_FREQ_04 5100
#define FILTER_CUTOUT_FREQ_05 2800

#define NO_COUNTS_FREQ 1000

#define PRESCALER 8
#define PRESCALER_RESET_PIN 11

int selectedFilter = 0;

LiquidCrystal lcd(LCD_RS_PIN, LCD_E_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN,
                  LCD_D7_PIN);

void selectFilters (unsigned long counterFreq);

void setup() {
  initPinsDefault();
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.print(F("wb6b Auto TX LPF"));
  lcd.setCursor(0, 1);
  lcd.print(F("V 0.1.1"));

  FreqCount.begin(10);
  selectDefaultFilter();

  delay(2000);
  lcd.clear();
}

void loop() {
  if (FreqCount.available()) {
    unsigned long rawCounter = FreqCount.read();
    unsigned long counterFreq = rawCounter * PRESCALER;
    counterFreq /= 10; // Limit to 1Khz steps.
    selectFilters(counterFreq);
    lcd.setCursor(0, 1);
    lcd.print(F("KHZ: "));

    lcd.print(counterFreq);
    lcd.print(F("       "));

    lcd.setCursor(0, 0);
    lcd.print(F("Filter: "));
    lcd.print(selectedFilter);
  }
}

void selectFilters (unsigned long counterFreq) {
  if (counterFreq < NO_COUNTS_FREQ) {
    int holdMode = digitalRead(HOLD_MODE_PIN);
    if (holdMode == true) {
      return;
    } else {
      selectDefaultFilter();
      return;
    }
  }

  if (counterFreq < FILTER_CUTOUT_FREQ_05) {
    selectedFilter = 05;
    digitalWrite(FILTER_ENB_PIN_05, PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_04, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_03, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_02, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_01, !PICK_LEVEL);
  } else if (counterFreq < FILTER_CUTOUT_FREQ_04) {
    selectedFilter = 04;
    digitalWrite(FILTER_ENB_PIN_05, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_04, PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_03, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_02, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_01, !PICK_LEVEL);
  } else if (counterFreq < FILTER_CUTOUT_FREQ_03) {
    selectedFilter = 03;
    digitalWrite(FILTER_ENB_PIN_05, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_04, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_03, PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_02, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_01, !PICK_LEVEL);
  } else if (counterFreq < FILTER_CUTOUT_FREQ_02) {
    selectedFilter = 02;
    digitalWrite(FILTER_ENB_PIN_05, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_04, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_03, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_02, PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_01, !PICK_LEVEL);
  } else if (counterFreq < FILTER_CUTOUT_FREQ_01) {
    selectedFilter = 01;
    digitalWrite(FILTER_ENB_PIN_05, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_04, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_03, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_02, !PICK_LEVEL);
    digitalWrite(FILTER_ENB_PIN_01, PICK_LEVEL);
  } else {
    selectDefaultFilter();
  }
}

void selectDefaultFilter() {
  selectedFilter = FILTER_DEFAULT_NUMBER;
  digitalWrite(FILTER_ENB_PIN_05, !PICK_LEVEL);
  digitalWrite(FILTER_ENB_PIN_04, !PICK_LEVEL);
  digitalWrite(FILTER_ENB_PIN_03, !PICK_LEVEL);
  digitalWrite(FILTER_ENB_PIN_02, !PICK_LEVEL);
  digitalWrite(FILTER_ENB_PIN_01, !PICK_LEVEL);
  digitalWrite(FILTER_DEFAULT_ENB_PIN, PICK_LEVEL);
}

void initPinsDefault() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT); // Frequency counter input pin
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, OUTPUT); // L led
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT);  // If I2C bus. Must be INPUT
  pinMode(A5, INPUT);  // If I2C bus. Must be INPUT
  pinMode(A6, INPUT_PULLUP);
  pinMode(A7, INPUT_PULLUP);

  // Setup output pins.
  pinMode(FILTER_ENB_PIN_01, OUTPUT);
  pinMode(FILTER_ENB_PIN_02, OUTPUT);
  pinMode(FILTER_ENB_PIN_03, OUTPUT);
  pinMode(FILTER_ENB_PIN_04, OUTPUT);
  pinMode(FILTER_ENB_PIN_05, OUTPUT);
  pinMode(PRESCALER_RESET_PIN, OUTPUT);

  // Reset prescaler
  digitalWrite(PRESCALER_RESET_PIN, false);
  delay(10);
  digitalWrite(PRESCALER_RESET_PIN, true);
}

