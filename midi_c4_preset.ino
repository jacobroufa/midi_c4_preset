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

volatile int _mode = 0;
volatile int _lastMode = 0;
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
  lcd.print("sending code ");
  lcd.print(val);
  delay(800);
  _updateDisplayByMode();
}

void hold() {}

void bfa_tap() {
  if (_mode == 0) {
    sendPresetCode(2 + (_presetPage * 4));
  }
  if (_mode == 1) {
    sendPresetCode(3);
  }
};
Button bfa_buttons[1] { b0 };
ButtonFns bfa(bfa_tap, hold, 1, bfa_buttons);

void bfb_tap() {
  if (_mode == 0) {
    sendPresetCode(3 + (_presetPage * 4));
  }
  if (_mode == 1) {
    sendPresetCode(4);
  }
};
Button bfb_buttons[1] { b1 };
ButtonFns bfb{ bfb_tap, hold, 1, bfb_buttons };

void bfc_tap() {
  if (_mode == 0) {
    _presetPage = getNewValue(false, 31, _presetPage);
  }
  if (_mode == 1) {
    // tap
  }
};
Button bfc_buttons[1] { b2 };
ButtonFns bfc{ bfc_tap, hold, 1, bfc_buttons };

void bfd_tap() {
  if (_mode == 0) {
    sendPresetCode(0 + (_presetPage * 4));
  }
  if (_mode == 1) {
    sendPresetCode(0);
  }
};
Button bfd_buttons[1] { b3 };
ButtonFns bfd{ bfd_tap, hold, 1, bfd_buttons };

void bfe_tap() {
  if (_mode == 0) {
    sendPresetCode(1 + (_presetPage * 4));
  }
  if (_mode == 1) {
    sendPresetCode(1);
  }
};
Button bfe_buttons[1] { b4 };
ButtonFns bfe{ bfe_tap, hold, 1, bfe_buttons };

void bff_tap() {
  if (_mode == 0) {
    _presetPage = getNewValue(true, 31, _presetPage);
  }
  if (_mode == 1) {
    sendPresetCode(2);
  }
};
Button bff_buttons[1] { b5 };
ButtonFns bff{ bff_tap, hold, 1, bff_buttons };

void bf2l_tap() {
  _mode = getNewValue(true, 1, _mode);
};
Button bf2l_buttons[2] { b0, b3 };
ButtonFns bf2l{ bf2l_tap, hold, 2, bf2l_buttons };

ButtonFns bcButtons[7] { bf2l, bfa, bfb, bfc, bfd, bfe, bff };
// initialize button controller
ButtonController bc(bcButtons, 7);

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

void displayPresetTap() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print((_presetPage * 5) + 1); // 0*5 + 1 = 1, 1*5 + 1 = 6, etc
  lcd.setCursor(9, 0);
  lcd.print((_presetPage * 5) + 2); // 0*5 + 2 = 2, 1*5 + 2 = 7, etc
  lcd.setCursor(18, 0);
  lcd.print((_presetPage * 5) + 3); // 0*5 + 3 = 3, 1*5 + 3 = 8, etc
  lcd.setCursor(0, 1);
  lcd.print((_presetPage * 5) + 4); // 0*5 + 4 = 4, 1*5 + 4 = 9, etc
  lcd.setCursor(9, 1);
  lcd.print((_presetPage * 5) + 5); // 0*5 + 5 = 5, 1*5 + 5 = 10, etc

  lcd.setCursor(17, 1);
  lcd.print("TAP");
}

void _updateDisplayByMode() {
  if (_mode == 0) {
    displayPresetScroll();
  }
  if (_mode == 1) {
    displayPresetTap();
  }
}

void updateDisplay() {
  if (_mode != _lastMode) {
    _presetPage = 0;
    _lastPresetPage = 0;
    _lastMode = _mode;

    _updateDisplayByMode();
  }

  if (_presetPage != _lastPresetPage) {
    _lastPresetPage = _presetPage;

    _updateDisplayByMode();
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
  for (byte i = 0; i < 7; i++) {
    for (byte j = 0; j < bcButtons[i].buttonsLn; j++) {
      bcButtons[i].buttons[j].loop();
    }
  }
  // update display when necessary
  updateDisplay();
}
