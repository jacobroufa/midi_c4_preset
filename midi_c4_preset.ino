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
#include <usbh_midi.h> // https://github.com/gdsports/USB_Host_Library_SAMD

#define DEBOUNCE 20

USBHost UsbH;
USBH_MIDI Midi(&UsbH);

#define DATAPIN 41
#define CLOCKPIN 40
Adafruit_DotStar dotstar = Adafruit_DotStar(1, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// button controller
#include "button.h"

const int LCD_0 = 0x27;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(LCD_0, 20, 2);

const int EXP_0 = A1; // expression

const int TAP_CC = 93;
const int EXP_CC = 100;
const int BYP_CC = 103;
const int ENG_CC = 104;

// leave this a const for now -- multi-channel device later
const int CHANNEL = 0;

volatile int _presetPage = 0;

Button b0(10); // upper left
Button b1(9); // upper middle
Button b2(7); // upper right
Button b3(3); // lower left
Button b4(4); // lower middle
Button b5(2); // lower right

void printDisplay(String value) {
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print(value);
  delay(300);
}

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

volatile int _currentChar = 0;
const char abc[36] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
char getNewAbc(bool up) {
  return abc[getNewValue(up, 35, _currentChar)];
}

volatile bool _editing = false;
volatile int _editStep = 0;
volatile int _editBtn = 0;
volatile int _chVal = 0;
volatile int _prsVal = 0;
String _nameVal = "";
void setEditing(int btn = 0) {
  _editing = !_editing;
  if (_editing) {
    _editBtn = btn;
    _editStep = 0;
    _chVal = 0;
    _prsVal = 0;
    _nameVal = "";
  }
}

void sendMidi(int cc, int value) {
  uint8_t buf[3] = {
    0xB0 | (CHANNEL & 0xf),
    cc & 0x7f,
    value & 0x7f
  };
  if (Midi) {
    Midi.SendData(buf);
  }
}

volatile long _expTime;
volatile int _oldExpVal;
void setExp() {
  const long now = millis();
  if ((now - _expTime) < DEBOUNCE) return;
  _expTime = now;

  // get the value from the expression input
  volatile int value = analogRead(EXP_0);
  // limit input outside this range
  value = constrain(value, 5, 1000);
  // convert it for MIDI values 0-127
  value = map(value, 5, 1000, 0, 127);

  // only send if we haven't yet
  if (value != _oldExpVal) {
    // cache value
    _oldExpVal = value;
    // send that value over the wire
    sendMidi(EXP_CC, value);
  }
}

volatile int _lastPreset;
void setPreset(int value) {
  if (value != _lastPreset) {
    _lastPreset = value;
    sendMidi(BYP_CC, value);
  }
}

void sendPresetCode(int value) {
  setPreset(value);
  printDisplay(String("preset ") + (value + 1));
  _updateDisplayByMode();
}

int getPresetNumber(int button) {
  return button + (_presetPage * 6);
}

volatile int _mode = 0;

void hold() {}

void bfa_tap() {
  if (_mode == 0) sendPresetCode(getPresetNumber(0));
  if (_mode == 1) {
    if (!_editing) setEditing(0);
    _updateDisplayByMode();
  }
};
Button bfa_buttons[1] { b0 };
ButtonFns bfa(bfa_tap, hold, 1, bfa_buttons);

void bfb_tap() {
  if (_mode == 0) sendPresetCode(getPresetNumber(1));
  if (_mode == 1) {
    if (!_editing) setEditing(1);
    _updateDisplayByMode();
  }
};
Button bfb_buttons[1] { b1 };
ButtonFns bfb{ bfb_tap, hold, 1, bfb_buttons };

void bfc_tap() {
  if (_mode == 0) sendPresetCode(getPresetNumber(2));
  if (_mode == 1) {
    if (!_editing) setEditing(2);
    _updateDisplayByMode();
  }
};
Button bfc_buttons[1] { b2 };
ButtonFns bfc{ bfc_tap, hold, 1, bfc_buttons };

void bfd_tap() {
  if (_mode == 0) sendPresetCode(getPresetNumber(3));
  if (_mode == 1) {
    if (!_editing) {
      setEditing(3);
    } else {
      setEditing(); // toggle off
    }
    _updateDisplayByMode();
  }
};
Button bfd_buttons[1] { b3 };
ButtonFns bfd{ bfd_tap, hold, 1, bfd_buttons };

void bfe_tap() {
  if (_mode == 0) sendPresetCode(getPresetNumber(4));
  if (_mode == 1) {
    if (!_editing) {
      setEditing(4);
    } else {
      if (_editStep != 3) {
        _editStep = getNewValue(true, 3, _editStep);
      }
    }
    _updateDisplayByMode();
  }
};
Button bfe_buttons[1] { b4 };
ButtonFns bfe{ bfe_tap, hold, 1, bfe_buttons };

void bff_tap() {
  if (_mode == 0) sendPresetCode(getPresetNumber(5));
  if (_mode == 1) {
    if (!_editing) {
      setEditing(5);
    } else {
      if (_editStep == 3) {
        // save
        printDisplay("saving");
        setEditing(); // toggle off
      }
    }
    _updateDisplayByMode();
  }
};
Button bff_buttons[1] { b5 };
ButtonFns bff{ bff_tap, hold, 1, bff_buttons };

// lower left and middle
void bfde_tap() {
  if (!_editing) {
    _presetPage = getNewValue(false, 21, _presetPage);
    printDisplay("page dn");
    _updateDisplayByMode();
  }
};
Button bfde_buttons[2] { b3, b4 };
ButtonFns bfde{ bfde_tap, hold, 2, bfde_buttons };

// lower middle and right
void bfef_tap() {
  if (!_editing) {
    _presetPage = getNewValue(true, 21, _presetPage);
    printDisplay("page up");
    _updateDisplayByMode();
  }
};
Button bfef_buttons[2] { b4, b5 };
ButtonFns bfef{ bfef_tap, hold, 2, bfef_buttons };

// lower left and upper right
void bfcd_tap() {
  _mode = getNewValue(true, 1, _mode);
  printDisplay("change mode");
  _updateDisplayByMode();
}
Button bfcd_buttons[2] { b2, b3 };
ButtonFns bfcd{ bfcd_tap, hold, 2, bfcd_buttons };

const int _bcBtnLn = 9;
ButtonFns bcButtons[_bcBtnLn] { bfde, bfef, bfcd, bfa, bfb, bfc, bfd, bfe, bff };
// initialize button controller
ButtonController bc(bcButtons, _bcBtnLn);

int alignM(int ln) {
  if (ln == 1) return 10;
  if (ln <= 3) return 9;
  if (ln <= 5) return 8;
  return 7;
}

int alignR(int ln) {
  if (ln == 1) return 19;
  if (ln == 2) return 18;
  if (ln == 3) return 17;
  if (ln == 4) return 16;
  if (ln == 5) return 15;
  return 14;
}

void setButtonDisplay(String values[6]) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(values[0]);
  lcd.setCursor(alignM(values[1].length()), 0);
  lcd.print(values[1]);
  lcd.setCursor(alignR(values[2].length()), 0);
  lcd.print(values[2]);
  lcd.setCursor(0, 1);
  lcd.print(values[3]);
  lcd.setCursor(alignM(values[4].length()), 1);
  lcd.print(values[4]);
  lcd.setCursor(alignR(values[5].length()), 1);
  lcd.print(values[5]);
}

void displayPresetScroll() {
  String presets[6];
  for (int i = 1; i <= 6; i++) {
    presets[i - 1] = String(getPresetNumber(i));
  }

  setButtonDisplay(presets);
}

void displaySettings() {
  String presets[6];

  if (!_editing) {
    presets[0] = "Edit preset bank " + String(_presetPage);
    presets[3] = "Press to set";
  } else {
    switch (_editStep) {
      case 0:
        presets[0] = "Chan";
        presets[1] = String(_chVal);
        presets[3] = "exit";
        presets[4] = "next";
        break;
      case 1:
        presets[0] = "Preset";
        presets[1] = String(_prsVal);
        presets[3] = "exit";
        presets[4] = "next";
        break;
      case 2:
        presets[0] = "Name";
        presets[1] = _nameVal;
        presets[3] = "exit";
        presets[4] = "next";
        break;
      case 3:
        presets[0] = String(_chVal) + " " + String(_prsVal);
        presets[1] = _nameVal;
        presets[3] = "exit";
        presets[5] = "confirm";
        break;
    }
  }

  setButtonDisplay(presets);
}

void _updateDisplayByMode() {
  switch (_mode) {
    case 0:
      displayPresetScroll();
      break;
    case 1:
      displaySettings();
      break;
  }
}

volatile long _dispTime;
volatile int _lastPresetPage = 0;
volatile int _lastMode = 0;
void updateDisplay() {
  const long now = millis();
  if ((now - _dispTime) < DEBOUNCE) return;
  _dispTime = now;

  if (
    (_presetPage != _lastPresetPage) ||
    (_mode != _lastMode)
  ) {
    _lastPresetPage = _presetPage;
    _lastMode = _mode;
    _updateDisplayByMode();
  }
}

void setup() {
  // initialize lcd
  lcd.init();
  lcd.backlight();
  displayPresetScroll();

  // initialize usb host
  if (UsbH.Init()) {
    lcd.clear();
    lcd.print("usb failed");
    while(1); // halt
  }

  // if LEDs are still on the USB port failed

  // turn off onboard red led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // turn off onboard rgb led
  dotstar.begin();
  dotstar.clear();
  dotstar.show();

  delay(200);
}

void loop() {
  // keep the usb active
  UsbH.Task();
  // run button loops
  for (byte i = 0; i < _bcBtnLn; i++) {
    for (byte j = 0; j < bcButtons[i].buttonsLn; j++) {
      bcButtons[i].buttons[j].loop();
    }
  }
  // run button controller
  bc.loop();
  // set expression values
  setExp();
  // update display when necessary
  updateDisplay();
}
