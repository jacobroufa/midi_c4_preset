/**
 * Button layout:
 * |-------|
 * |   0   |
 * | 1   2 |
 * |-------|
 *
 * Settings:
 * - midi channel: 0-15
 * - tap button: 0-2
 * - favorites: 0-127[]
 * - expression cc: 0-127
 */
#include <vector>
#include "button.h"

const string MODE_0 = "FAVORITES"; // all three buttons map to preset favorites
const string MODE_1 = "FAV_TAP"; // buttons 0 and 1 map to preset favorites; button 2 is tap
const string MODE_2 = "PRESET_TAP"; // buttons 0 and 1 scroll through presets; button 2 is tap
const string MODE_3 = "SETTINGS": // set midi channel, tap button, presets, expression cc

const int BUTTON_0 = 0;
const int BUTTON_1 = 1;
const int BUTTON_2 = 2;

const Button b0(BUTTON_0);
const Button b1(BUTTON_1);
const Button b2(BUTTON_2);

void bf3_tap() { /** do something w/ 3 buttons tap */ };
void bf3_hold() { /** do something w/ 3 buttons hold */ };
const ButtonFns bf3{ bf3_tap, bf3_hold, std::vector<Button> {b0, b1, b2} };

void bf2a_tap() { /** do something w/ 2 buttons tap */ };
void bf2a_hold() { /** do something w/ 2 buttons hold */ };
const ButtonFns bf2a{ bf2a_tap, bf2a_hold, std::vector<Button> {b0, b1} };

void bf2b_tap() { /** do something w/ 2 buttons tap */ };
void bf2b_hold() { /** do something w/ 2 buttons hold */ };
const ButtonFns bf2b{ bf2b_tap, bf2b_hold, std::vector<Button> {b0, b2} };

void bf2c_tap() { /** do something w/ 2 buttons tap */ };
void bf2c_hold() { /** do something w/ 2 buttons hold */ };
const ButtonFns bf2c{ bf2c_tap, bf2c_hold, std::vector<Button> {b1, b2} };

void bfa_tap() { /** do something w/ 1 buttons tap */ };
void bfa_hold() { /** do something w/ 1 buttons hold */ };
const ButtonFns bfa{ bfa_tap, bfa_hold, std::vector<Button> {b0} };

void bfb_tap() { /** do something w/ 1 buttons tap */ };
void bfb_hold() { /** do something w/ 1 buttons hold */ };
const ButtonFns bfb{ bfb_tap, bfb_hold, std::vector<Button> {b1} };

void bfc_tap() { /** do something w/ 1 buttons tap */ };
void bfc_hold() { /** do something w/ 1 buttons hold */ };
const ButtonFns bfc{ bfc_tap, bfc_hold, std::vector<Button> {b2} };

const std::vector<ButtonFns> bcButtons { bf3, bf2a, bf2b, bf2c, bfa, bfb, bfc };
ButtonController bc(bcButtons);

void setup() {}

void loop() {
  bc.loop();
}
