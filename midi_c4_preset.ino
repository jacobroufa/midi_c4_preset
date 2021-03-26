/**
 * |------------|
 * | UL  UM  UR |
 * |  [||||||]  |
 * | LL  LM  LR |
 * |------------|
 *
 * Settings:
 * - midi channel: 0-15
 * - tap button: 0-5
 * - favorites: 0-127[]
 * - expression cc: 0-127
 */

#include <Wire.h> // I2C communication
#include <LiquidCrystal_I2C.h> // LCD
#include <Adafruit_DotStar.h> // https://github.com/adafruit/Adafruit_DotStar

#define DATAPIN 41
#define CLOCKPIN 40
Adafruit_DotStar dotstar = Adafruit_DotStar(1, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// button controller
#include "button.h"

const int LCD_0 = 0x27;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(LCD_0, 20, 2);

const int BTN_0 = 3; // lower left
const int BTN_1 = 4; // lower middle
const int BTN_2 = 2; // lower right
const int BTN_3 = 10; // upper left
const int BTN_4 = 9; // upper middle
const int BTN_5 = 7; // upper right
// const int EXP_0 = A1; // expression

volatile int _presetPage = 0;
volatile int _lastPresetPage = 0;

Button b0(BTN_0);
Button b1(BTN_1);
Button b2(BTN_2);
Button b3(BTN_3);
Button b4(BTN_4);
Button b5(BTN_5);

int getNewValue(bool up, int max, int value) {
  volatile int newValue = value;

  if (up) {
    newValue += 1;

    return (newValue > max) ? 0 : newValue;
  }

  // else down
  newValue -= 1;

  return (newValue < 0) ? max : newValue;
}

void sendPresetCode(int val) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("setting preset ");
  lcd.print(val + 1);
  delay(600);
  displayPresetScroll();
}

void hold() {}

void bfa_tap() {
  sendPresetCode(2 + (_presetPage * 4));
};
Button bfa_buttons[1] { b0 };
ButtonFns bfa(bfa_tap, hold, 1, bfa_buttons);

void bfb_tap() {
  sendPresetCode(3 + (_presetPage * 4));
};
Button bfb_buttons[1] { b1 };
ButtonFns bfb{ bfb_tap, hold, 1, bfb_buttons };

void bfc_tap() {
  _presetPage = getNewValue(false, 31, _presetPage);
};
Button bfc_buttons[1] { b2 };
ButtonFns bfc{ bfc_tap, hold, 1, bfc_buttons };

void bfd_tap() {
  sendPresetCode(0 + (_presetPage * 4));
};
Button bfd_buttons[1] { b3 };
ButtonFns bfd{ bfd_tap, hold, 1, bfd_buttons };

void bfe_tap() {
  sendPresetCode(1 + (_presetPage * 4));
};
Button bfe_buttons[1] { b4 };
ButtonFns bfe{ bfe_tap, hold, 1, bfe_buttons };

void bff_tap() {
  _presetPage = getNewValue(true, 31, _presetPage);
};
Button bff_buttons[1] { b5 };
ButtonFns bff{ bff_tap, hold, 1, bff_buttons };

ButtonFns bcButtons[6] { bfa, bfb, bfc, bfd, bfe, bff };
// initialize button controller
ButtonController bc(bcButtons, 6);

void displayPresetScroll() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print((_presetPage * 4) + 1); // 0*4 + 1 = 1, 1*4 + 1 = 5, etc
  lcd.setCursor(9, 0);
  lcd.print((_presetPage * 4) + 2); // 0*4 + 2 = 2, 1*4 + 2 = 6, etc
  lcd.setCursor(0, 1);
  lcd.print((_presetPage * 4) + 3); // 0*4 + 3 = 3, 1*4 + 3 = 7, etc
  lcd.setCursor(9, 1);
  lcd.print((_presetPage * 4) + 4); // 0*4 + 4 = 4, 1*4 + 4 = 8, etc

  lcd.setCursor(18, 0);
  lcd.print("UP");
  lcd.setCursor(16, 1);
  lcd.print("DOWN");
}

void updateDisplay() {
  if (_presetPage != _lastPresetPage) {
    _lastPresetPage = _presetPage;

    displayPresetScroll();
  }
}

void setup() {
  // Serial.begin(9600);

  // turn off onboard rgb led
  dotstar.begin();
  dotstar.clear();
  dotstar.show();

  // turn off onboard red led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // initialize lcd
  lcd.init();
  lcd.backlight();
  displayPresetScroll();
}

void loop() {
  // run button controller
  bc.loop();
  // run button loops
  for (byte i = 0; i < 6; i++) {
    for (byte j = 0; j < bcButtons[i].buttonsLn; j++) {
      bcButtons[i].buttons[j].loop();
    }
  }
  // update display when necessary
  updateDisplay();
}
