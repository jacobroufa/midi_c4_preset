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

Button b0(BTN_0);
Button b1(BTN_1);
Button b2(BTN_2);
Button b3(BTN_3);
Button b4(BTN_4);
Button b5(BTN_5);

void bfa_tap() {
  lcd.clear();
  lcd.print("LL -- TAP");
};
void bfa_hold() {
  lcd.clear();
  lcd.print("LL -- HOLD");
};
Button bfa_buttons[1] { b0 };
ButtonFns bfa(bfa_tap, bfa_hold, 1, bfa_buttons);

void bfb_tap() {
  lcd.clear();
  lcd.print("LM -- TAP");
};
void bfb_hold() {
  lcd.clear();
  lcd.print("LM -- HOLD");
};
Button bfb_buttons[1] { b1 };
ButtonFns bfb{ bfb_tap, bfb_hold, 1, bfb_buttons };

void bfc_tap() {
  lcd.clear();
  lcd.print("LR -- TAP");
};
void bfc_hold() {
  lcd.clear();
  lcd.print("LR -- HOLD");
};
Button bfc_buttons[1] { b2 };
ButtonFns bfc{ bfc_tap, bfc_hold, 1, bfc_buttons };

void bfd_tap() {
  lcd.clear();
  lcd.print("UL -- TAP");
};
void bfd_hold() {
  lcd.clear();
  lcd.print("UL -- HOLD");
};
Button bfd_buttons[1] { b3 };
ButtonFns bfd{ bfd_tap, bfd_hold, 1, bfd_buttons };

void bfe_tap() {
  lcd.clear();
  lcd.print("UM -- TAP");
};
void bfe_hold() {
  lcd.clear();
  lcd.print("UM -- HOLD");
};
Button bfe_buttons[1] { b4 };
ButtonFns bfe{ bfe_tap, bfe_hold, 1, bfe_buttons };

void bff_tap() {
  lcd.clear();
  lcd.print("UR -- TAP");
};
void bff_hold() {
  lcd.clear();
  lcd.print("UR -- HOLD");
};
Button bff_buttons[1] { b5 };
ButtonFns bff{ bff_tap, bff_hold, 1, bff_buttons };

void bf2l_tap() {
  lcd.clear();
  lcd.print("LEFT -- TAP");
};
void bf2l_hold() {
  lcd.clear();
  lcd.print("LEFT -- HOLD");
};
Button bf2l_buttons[2] { b0, b3 };
ButtonFns bf2l{ bf2l_tap, bf2l_hold, 2, bf2l_buttons };

void bf2c_tap() {
  lcd.clear();
  lcd.print("CENTER -- TAP");
};
void bf2c_hold() {
  lcd.clear();
  lcd.print("CENTER -- HOLD");
};
Button bf2c_buttons[2] { b1, b4 };
ButtonFns bf2c{ bf2c_tap, bf2c_hold, 2, bf2c_buttons };

void bf2r_tap() {
  lcd.clear();
  lcd.print("RIGHT -- TAP");
};
void bf2r_hold() {
  lcd.clear();
  lcd.print("RIGHT -- HOLD");
};
Button bf2r_buttons[2] { b2, b5 };
ButtonFns bf2r{ bf2r_tap, bf2r_hold, 2, bf2r_buttons };

ButtonFns bcButtons[9] { bf2l, bf2c, bf2r, bfa, bfb, bfc, bfd, bfe, bff };
// initialize button controller
ButtonController bc(bcButtons, 9);

void setup() {
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
  lcd.print("MIDI Controller");
}

void loop() {
  // run button controller
  bc.loop();
}
