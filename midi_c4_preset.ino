#include <TM1637Display.h>
#include <usbmidi.h>

const int BUTTON_UP = 2;
const int BUTTON_DOWN = 3;
const int BUTTON_DEBOUNCE = 20;

const int LCD_DIO = 4;
const int LCD_CLK = 5;

const int CHANNEL_DIP = 4;
const int DIP_DEBOUNCE = 5000; // 5s in between reading dip
const int DIP[CHANNEL_DIP] = { 6, 7, 8, 9 };
const bool dipValues[CHANNEL_DIP];

volatile int channel = 0; // maybe this is 1?
volatile int commandCode = 104; // 103 sends disabled

volatile int preset = 0;

volatile long buttonTime = 0;
volatile long dipTime = 0;
volatile long displayTime = 0;

// instantiate display
TM1637Display display(LCD_CLK, LCD_DIO);

void changePreset(int value) {
  USBMIDI.write(0xB0 | (channel & 0xf));
  USBMIDI.write(commandCode & 0x7f);
  USBMIDI.write(value & 0x7f);
}

// from https://stackoverflow.com/a/32410344
int getChannel() {
  int newChannel = 0;
  int tmp;

  for (int i = 0; i < CHANNEL_DIP; i++) {
    tmp = getValue(DIP[i]);
    newChannel |= tmp << (CHANNEL_DIP - i - 1);
  }

  return newChannel;
}

int getNewPreset(bool up) {
  int max = 127;
  int newPreset = preset;

  if (up) {
    newPreset += 1;

    if (newPreset > max) {
      return 0;
    }

    return newPreset;
  }

  // else down
  newPreset -= 1;

  if (newPreset < 0) {
    return max;
  }

  return newPreset;
}

bool getValue(int pin) {
  return digitalRead(pin) == 0;
}

void readButtons() {
  if ((millis() - buttonTime) < BUTTON_DEBOUNCE) {
    return;
  }

  int prevPreset = preset;
  volatile int newPreset = preset;

  bool upPressed = getValue(BUTTON_UP);
  bool downPressed = getValue(BUTTON_DOWN);

  if (upPressed) {
    newPreset = getNewPreset(true);
  }
  if (downPressed) {
    newPreset = getNewPreset(false);
  }

  if (prevPreset != newPreset) {
    changePreset(newPreset);
  }

  buttonTime = millis();
}

void readDip() {
  if ((millis() - dipTime) < DIP_DEBOUNCE) {
    return;
  }

  channel = getChannel();

  dipTime = millis();
}

void updateDisplay() {
  if ((millis() - displayTime) < (BUTTON_DEBOUNCE * 2)) {
    return;
  }

  display.showNumberDec(preset, false);

  displayTime = millis();
}

void setup() {
  channel = getChannel();

  display.clear();
  display.setBrightness(4);
}

void loop() {
  USBMIDI.poll();

  while (USBMIDI.available()) {
    USBMIDI.read();
  }

  readButtons();
  readDip();
  updateDisplay();

  USBMIDI.flush();
}
