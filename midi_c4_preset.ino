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
/**
 * for a given displayed page (6 buttons)
 * for each button
 * declare (read?): struct Preset bNpreset
 * if (bNpreset.mode & MODE_MSG) // do msg things
 * bNpreset.name
 */

#include <Wire.h> // I2C communication
#include <LiquidCrystal_I2C.h> // LCD
#include <Adafruit_DotStar.h> // https://github.com/adafruit/Adafruit_DotStar
#include <usbh_midi.h> // https://github.com/gdsports/USB_Host_Library_SAMD
#include <usbhub.h>

#include "button.h" // button controller

#define DEBOUNCE 20
#define EEPROM 0x50

#define MODE_MSG 0x00
#define MODE_TAP 0x01
#define MODE_PUP 0x02
#define MODE_PDN 0x03

#define HALF_BYTE 0x7F // 127
#define PRESET_SIZE 13

struct Preset {
  byte mode:  2;
  byte m1:    1;
  byte m2:    1;
  byte expch: 4; // byte 0
  byte exp:   1;
  byte expcc: 7; // byte 1
  byte m1ch:  4;
  byte m2ch:  4; // byte 2
  byte m1cc:  8; // byte 3
  byte m2cc:  8; // byte 4
  byte m1val: 8; // byte 5
  byte m2val: 8; // byte 6
  char namea: 8; // byte 7
  char nameb: 8; // byte 8
  char namec: 8; // byte 9
  char named: 8; // byte 10
  char namee: 8; // byte 11
  char namef: 8; // byte 12
};

USBHost UsbH; // create USB
USBH_MIDI MidiUsbH(&UsbH); // create Midi
Adafruit_DotStar dotstar = Adafruit_DotStar(1, 41, 40, DOTSTAR_BRG); // create dotstar led
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 2); // create lcd

Button b0(10); // upper left
Button b1(9); // upper middle
Button b2(7); // upper right
Button b3(3); // lower left
Button b4(4); // lower middle
Button b5(2); // lower right

const int EXP_0 = A1; // expression
const int TAP_CC = 93;
const int EXP_CC = 100;
const int BYP_CC = 103;
const int ENG_CC = 104;
// const char abc[62] = {'A', 'a', 'B', 'b', 'C', 'c', 'D', 'd', 'E', 'e', 'F', 'f', 'G', 'g', 'H', 'h', 'I', 'i', 'J', 'j', 'K', 'k', 'L', 'l', 'M', 'm', 'N', 'n', 'O', 'o', 'P', 'p', 'Q', 'q', 'R', 'r', 'S', 's', 'T', 't', 'U', 'u', 'V', 'v', 'W', 'w', 'X', 'x', 'Y', 'y', 'Z', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const int _bcBtnLn = 9; // number of button groups to control

volatile int _presetPage = 0;
volatile int _presetStartAddress = 0;
volatile char _abcChar = 97; // a
volatile bool _editing = false;
volatile int _editStep = 0;
volatile int _editBtn = 0;
volatile int _chVal = 0;
volatile int _prsVal = 0;
char _nameVal[6];
volatile int _nameChar = 0;
volatile long _expTime;
volatile int _mode = 0;
volatile long _dispTime;

Preset* currentPresets[6];
Preset* currentPreset;

// display something in the upper middle of the screen for 300ms
void printDisplay(String value, bool doDelay=true) {
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print(value);
  if (doDelay) delay(150);
}

// cycle a value up or down within a defined 0-max range
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

// cycle through our abc character array
char getNewAbc(bool up) {
  // _abcChar = getNewValue(up, 61, _abcChar);
  // return abc[_abcChar];
  if (up) {
    if (_abcChar == 122) return 65; // z up = A 
    if (_abcChar == 90) return 97; // Z up = a
    return _abcChar + 1;
  }
  if (_abcChar == 97) return 90; // a down = Z 
  if (_abcChar == 65) return 122; // A down = z
  return _abcChar - 1;
}

// set the editing flag in settings mode
void setEditing(int btn, bool editing) {
  if (_mode != 1) return; // don't do anything if we're not in settings mode
  _editing = editing;
  if (_editing) { // if we are toggling on, set default values for all the necessary editables
    _editBtn = btn;
    _editStep = 0;
    _nameChar = 0;
    _abcChar = 97;
    _chVal = 0; // bank[btn].chval || 0
    _prsVal = 0; // bank[btn].prsval || 0
    _nameVal[0] = 0; // bank[btn].nameval || 0
  }
}

byte eeprom_read_byte(long eeaddress) {
  Wire.beginTransmission(EEPROM);

  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();

  Wire.requestFrom(EEPROM, 1);

  byte rdata = 0x00;
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

void eeprom_write_byte(long eeaddress, byte data) {
  Wire.beginTransmission(EEPROM);

  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
}

void eepromRead(int addr, Preset* output, int length) {
    unsigned char *dst = (unsigned char *)&output;
    int address = addr;
    for (byte i = 0; i < length; i++) {
        *dst++ = eeprom_read_byte(address++);
    }
}

void eepromWrite(int addr, Preset input, int length) {
  unsigned char *src = (unsigned char *)&input;
  int address = addr;
  for (byte i = 0; i < length; i++) {
    eeprom_write_byte(address++, *src++);
  }
}

void setCurrentPresets(int page) {
  const byte numButtons = 6;
  _presetStartAddress = page * numButtons * PRESET_SIZE;

  for (byte i = 0; i < numButtons; i++) {
    int addr = _presetStartAddress + (i * PRESET_SIZE);
    Preset *cp = currentPresets[i];
    eepromRead(addr, cp, sizeof(Preset));
  }
}

void saveEditingPreset() {
  Preset newPreset;
  newPreset.mode = MODE_MSG;
  newPreset.namea = _nameVal[0];
  newPreset.nameb = _nameVal[1];
  newPreset.namec = _nameVal[2];
  newPreset.named = _nameVal[3];
  newPreset.namee = _nameVal[4];
  newPreset.namef = _nameVal[5];
  newPreset.exp = true;
  newPreset.expch = _chVal;
  newPreset.expcc = EXP_CC;
  newPreset.m1 = true;
  newPreset.m1ch = _chVal;
  newPreset.m1cc = ENG_CC;
  newPreset.m1val = _prsVal;

  int addr = _presetStartAddress + (_editBtn * PRESET_SIZE);
  eepromWrite(addr, newPreset, sizeof(Preset));
}

// send a value to a defined MIDI CC
void sendMidi(byte ch, byte cc, byte value, bool isCC) {
  byte message = isCC ? 0xB0 : 0xC0; 
  uint8_t buf[3];
  buf[0] = message | (ch & 0xf);
  buf[1] = cc & 0xf;
  if (isCC) {
    buf[2] = value & 0xf;
  }
  if (MidiUsbH) {
    MidiUsbH.SendData(buf);
  } else {
    printf("no/bad midi instance\n");
  }
}

volatile int _lastExpVal;
// set values from an expression pedal
void setExp() {
  Preset cp = *currentPreset;
  if (_mode != 0 || !cp.exp) return; // don't set expression values outside of our standard mode
  const long now = millis();
  if ((now - _expTime) < DEBOUNCE) return;
  _expTime = now;

  // get the value from the expression input
  volatile int value = analogRead(EXP_0);
  // only send if we haven't yet
  if (value != _lastExpVal) {
    // limit input outside this range
    value = constrain(value, 5, 1000);
    // convert it for MIDI values 0-127
    value = map(value, 5, 1000, 0, 127);
    // cache value
    _lastExpVal = value;
    // send that value over the wire
    byte cc = cp.expcc;
    byte ch = cp.expch;
    sendMidi(ch, cc, value, false);
  }
}

volatile short _lastPreset;
// set values from a button press
void setPresetCode(byte value) {
  if (_mode != 0) return; // don't set expression values outside of our standard mode
  short _newPreset = getPresetNumber(value);
  if (_newPreset != _lastPreset) {
    _lastPreset = _newPreset;
    currentPreset = currentPresets[value];
    Preset cp = *currentPreset;
    if (cp.m1) {
      sendMidi(
        cp.m1ch,
        HALF_BYTE & cp.m1cc,
        cp.m1val,
        cp.m1cc <= HALF_BYTE
      );
    }
    if (cp.m2) {
      sendMidi(
        cp.m2ch,
        HALF_BYTE & cp.m2cc,
        cp.m2val,
        cp.m2cc <= HALF_BYTE
      );
    }
    char name[6] = {
      cp.namea,
      cp.nameb,
      cp.namec,
      cp.named,
      cp.namee,
      cp.namef,
    };
    printDisplay(String(name));
  }
  _updateDisplayByMode();
}

// get the current value of a button based on current page
short getPresetNumber(byte button) {
  return button + (_presetPage * 6);
}

// align a String value in the middle of the screen based on its length
int alignM(int ln) {
  if (ln == 1) return 10;
  if (ln <= 3) return 9;
  if (ln <= 5) return 8;
  return 7;
}

// align a String value to the right of the screen based on its length
int alignR(int ln) {
  if (ln == 1) return 19;
  if (ln == 2) return 18;
  if (ln == 3) return 17;
  if (ln == 4) return 16;
  if (ln == 5) return 15;
  return 14;
}

// set an array of string values to the screen
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

void hold() {} // intentional no-op until we can get "hold" working well

// upper left
void bfa_tap() {
  if (_mode == 0) setPresetCode(0);
  if (_mode == 1) {
    if (!_editing) setEditing(0, true); // edit button 0
    if (_editing && _editStep == 2) { // abc left
      _abcChar = 97;
      _nameChar = getNewValue(false, 5, _nameChar);
    }
  }
};

// upper middle
void bfb_tap() {
  if (_mode == 0) setPresetCode(1);
  if (_mode == 1) {
    if (!_editing) setEditing(1, true); // edit button 1
    if (_editing && _editStep == 2) { // abc right
      _abcChar = 97;
      _nameChar = getNewValue(true, 5, _nameChar);
    }
  }
};

// upper right
void bfc_tap() {
  if (_mode == 0) setPresetCode(2);
  if (_mode == 1) {
    if (!_editing) setEditing(2, true); // edit button 2
    if (_editing && _editStep == 0) { _chVal = getNewValue(true, 15, _chVal); } // channel up
    if (_editing && _editStep == 1) { _prsVal = getNewValue(true, 127, _prsVal); } // preset up
    if (_editing && _editStep == 2) { _nameVal[_nameChar] = getNewAbc(true); } // abc up
  }
};

// lower left
void bfd_tap() {
  if (_mode == 0) setPresetCode(3);
  if (_mode == 1) {
    if (!_editing) setEditing(3, true); // edit button 3
    if (_editing) setEditing(0, false); // cancel edit mode
  }
};

// lower middle
void bfe_tap() {
  if (_mode == 0) setPresetCode(4);
  if (_mode == 1) {
    if (!_editing) setEditing(4, true); // edit button 4
    if (_editStep != 3) {
      _editStep = getNewValue(true, 3, _editStep); // next step
    } else {
      saveEditingPreset();
      printDisplay("saving");
      setEditing(0, false); // leave edit mode
    }
  }
};

// lower right
void bff_tap() {
  if (_mode == 0) setPresetCode(5);
  if (_mode == 1) {
    if (!_editing) setEditing(5, true); // edit button 5
    if (_editing && _editStep == 0) { _chVal = getNewValue(false, 15, _chVal); } // channel down
    if (_editing && _editStep == 1) { _prsVal = getNewValue(false, 127, _prsVal); } // preset down
    if (_editing && _editStep == 2) { _nameVal[_nameChar] = getNewAbc(false); } // abc down
  }
};

// lower left and middle
void bfde_tap() {
  if (_editing) return;
  _presetPage = getNewValue(false, 29, _presetPage);
  setCurrentPresets(_presetPage);
  printDisplay("page dn");
};

// lower middle and right
void bfef_tap() {
  if (_editing) return;
  _presetPage = getNewValue(true, 29, _presetPage);
  setCurrentPresets(_presetPage);
  printDisplay("page up");
};

// lower left and upper right
void bfcd_tap() {
  _mode = getNewValue(true, 1, _mode);
  _chVal = 0;
  _prsVal = 0;
  _nameChar = 0;
  _abcChar = 97;
  printDisplay("change mode");
}

Button bfa_buttons[1] { b0 };
ButtonFns bfa(bfa_tap, hold, 1, bfa_buttons);
Button bfb_buttons[1] { b1 };
ButtonFns bfb{ bfb_tap, hold, 1, bfb_buttons };
Button bfc_buttons[1] { b2 };
ButtonFns bfc{ bfc_tap, hold, 1, bfc_buttons };
Button bfd_buttons[1] { b3 };
ButtonFns bfd{ bfd_tap, hold, 1, bfd_buttons };
Button bfe_buttons[1] { b4 };
ButtonFns bfe{ bfe_tap, hold, 1, bfe_buttons };
Button bff_buttons[1] { b5 };
ButtonFns bff{ bff_tap, hold, 1, bff_buttons };
Button bfde_buttons[2] { b3, b4 };
ButtonFns bfde{ bfde_tap, hold, 2, bfde_buttons };
Button bfef_buttons[2] { b4, b5 };
ButtonFns bfef{ bfef_tap, hold, 2, bfef_buttons };
Button bfcd_buttons[2] { b2, b3 };
ButtonFns bfcd{ bfcd_tap, hold, 2, bfcd_buttons };
// create array of buttons to control
ButtonFns bcButtons[_bcBtnLn] { bfde, bfef, bfcd, bfa, bfb, bfc, bfd, bfe, bff };
// initialize button controller
ButtonController bc(bcButtons, _bcBtnLn);

// display preset numbers based on current page
void displayPresetScroll() {
  String presets[6];
  for (int i = 1; i <= 6; i++) {
    Preset cp = *currentPresets[i];
    char name[6] = {
      cp.namea,
      cp.nameb,
      cp.namec,
      cp.named,
      cp.namee,
      cp.namef,
    };
    presets[i - 1] = String(name);
  }

  setButtonDisplay(presets);
}

// display setting screen values based on whether we are editing a preset and which step we're on
void displaySettings() {
  String presets[6];

  if (!_editing) {
    presets[0] = "Edit preset bank " + String(_presetPage + 1);
    presets[3] = "Press to set";
  } else {
    presets[2] = String("Btn ") + (_editBtn + 1);
    if (_editStep == 0) {
      presets[0] = "Chan";
      presets[1] = String(_chVal);
      presets[3] = "exit";
      presets[4] = "next";
    } else if (_editStep == 1) {
      presets[0] = "Preset";
      presets[1] = String(_prsVal);
      presets[3] = "exit";
      presets[4] = "next";
    } else if (_editStep == 2) {
      presets[0] = "Name";
      presets[1] = String(_nameVal);
      presets[3] = "exit";
      presets[4] = "next";
    } else {
      presets[0] = "C" + String(_chVal) + " P" + String(_prsVal);
      presets[1] = String(_nameVal);
      presets[3] = "exit";
      presets[4] = "confirm";
    }
  }

  setButtonDisplay(presets);
}

// display presets or settings -- used internally to force rerender
void _updateDisplayByMode() {
  if (_mode != 1) return displayPresetScroll();
  displaySettings();
}

volatile int _lastPresetPage = -1;
volatile int _lastMode = -1;
volatile bool _lastEditing = true;
volatile int _lastEditStep = -1;
volatile int _lastChVal = -1;
volatile int _lastPrsVal = -1;
volatile int _lastNameChar = -1;
volatile char _lastAbcChar = 97 - 1;
// update our display if page or mode has changed -- used only in the loop
void updateDisplay() {
  const long now = millis();
  if ((now - _dispTime) < DEBOUNCE) return;
  _dispTime = now;

  if (
    (_presetPage != _lastPresetPage) ||
    (_mode != _lastMode) ||
    (_editing != _lastEditing) ||
    (_editStep != _lastEditStep) ||
    (_chVal != _lastChVal) ||
    (_prsVal != _lastPrsVal) ||
    (_nameChar != _lastNameChar) ||
    (_abcChar != _lastAbcChar)
  ) {
    _lastPresetPage = _presetPage;
    _lastMode = _mode;
    _lastEditing = _editing;
    _lastEditStep = _editStep;
    _lastChVal = _chVal;
    _lastPrsVal = _prsVal;
    _lastNameChar = _nameChar;
    _lastAbcChar = _abcChar;
    _updateDisplayByMode();
  }
}

// Function that printf and related will use to print
// int serial_putchar(char c, FILE* f) {
   // if (c == '\n') serial_putchar('\r', f);
   // return Serial1.write(c) == 1? 0 : 1;
// }

// FILE serial_stdout;

void setup() {
  Serial1.begin(9600);

  // TODO: figure out how to do get stdout to our serial port for logging the usbhost class
  // fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  // stdout = &serial_stdout;

  printf("start debugging\n");

  // initialize lcd
  lcd.init();
  lcd.backlight();

  // initialize usb host
  if (UsbH.Init()) {
    printf("usb failed to initialize\n");
    lcd.clear();
    lcd.print("usb failed");
    while(1); // halt
  }

  setCurrentPresets(_presetPage);

  // if LEDs are still on the USB port failed
  // turn off onboard red led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  // turn off onboard rgb led
  dotstar.begin();
  dotstar.clear();
  dotstar.show();

  printDisplay("SA MIDI");
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
