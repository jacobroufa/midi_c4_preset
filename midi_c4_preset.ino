#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

#include <TM1637Display.h> // https://github.com/avishorp/TM1637
// #include <usbh_midi.h> // https://github.com/gdsports/USB_Host_Library_SAMD

#ifdef ADAFRUIT_TRINKET_M0
// setup Dotstar LED on Trinket M0
#include <Adafruit_DotStar.h> // https://github.com/adafruit/Adafruit_DotStar
#define DATAPIN 7
#define CLOCKPIN 8
Adafruit_DotStar strip = Adafruit_DotStar(1, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
#endif

Adafruit_USBD_MIDI usb;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb, MIDI);

// USBHost usb;
// USBH_MIDI Midi(&usb);

const int BUTTON_UP = 1;
const int BUTTON_DOWN = 0;
const int BUTTON_DEBOUNCE = 140; // Any shorter and we get double-presses. Any longer it's sluggish feeling...
const int CHANNEL_DEBOUNCE = 1000; // Show "ch00" on the screen for 1000ms

const int commandCode = 103; // Bypass
// volatile int commandCode = 104; // Engaged
const int tapCode = 93;

volatile int channel = 0; // MIDI channel is 0 (displayed as 1) by default
volatile int preset = 0; // Initial preset value
volatile int mode = 0; // { preset, tap }

volatile long lastTapTime = 0;
volatile int tapCount = 0;
volatile long bpm = 0;

volatile long buttonTime = 0;
volatile long displayTime = 0;

// instantiate display
#ifdef ADAFRUIT_TRINKET_M0
const int LCD_DIO = 2; // 2; if trinket
const int LCD_CLK = 3; // 3; if trinket
#else
const int LCD_DIO = 5; // 2; if trinket
const int LCD_CLK = 4; // 3; if trinket
#endif
TM1637Display display(LCD_CLK, LCD_DIO);

int getNewValue(bool up, int max, int value) {
  volatile int newValue = value;

  if (up) {
    newValue += 1;

    if (newValue > max) {
      return 0;
    }

    return newValue;
  }

  // else down
  newValue -= 1;

  if (newValue < 0) {
    return max;
  }

  return newValue;
}

int getNewPreset(bool up) {
  int max = 127;
  return getNewValue(up, max, preset);
}

int getNewChannel() {
  int max = 15;
  return getNewValue(true, max, channel);
}

int getNewMode() {
  int max = 1;
  return getNewValue(true, max, mode);
}

bool getValue(int pin) {
  return digitalRead(pin) == LOW;
}

void showTap(uint8_t lastSeg) {
  uint8_t tapSeg[] = {
    SEG_A | SEG_B | SEG_C, // "T"
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, // "A"
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G, // "P"
    lastSeg
  };

  display.setSegments(tapSeg);
}

void sendBpm() {
  /** uint8_t buf[3] = {
    0xB0 | (channel & 0xf),
    tapCode & 0x7f,
    bpm & 0x7f
  };
  Midi.SendData(buf); **/
  MIDI.sendControlChange(tapCode, bpm, channel);
}

void setBpm(long lastTime, long currentTime) {
  const long MS_PER_MINUTE = 60000;
  const long beatMillis = currentTime - lastTime;
  const long newBpm = floor(MS_PER_MINUTE / beatMillis);

  if (newBpm < 40) {
    bpm = 0;
  }
  if (newBpm > 220) {
    bpm = 127;
  }
  bpm = ceil(((newBpm - 40) * 127) / 180);

  sendBpm();
}

void handleButtonTap() {
  const int MIN_TAPS = 3;
  const long MAX_TAP = 2000;
  const long now = millis();
  const bool resetTaps = now - lastTapTime > MAX_TAP && tapCount >= 0;

  showTap(SEG_C); // "."

  tapCount = resetTaps ? 0 : tapCount + 1;

  if (tapCount >= MIN_TAPS) {
    setBpm(lastTapTime, now);
  }

  lastTapTime = now;

  showTap(0); // return the screen to the state without a "." at the end connoting the tap
}

void useTapButtons(bool upPressed, bool downPressed) {
  if (upPressed) {
    handleButtonTap();
  }
  if (downPressed) {
    mode = 0; // back to preset
    // showPreset();
  }
}

void showPreset() {
  display.showNumberDec(preset + 1);
}

void changePreset(int value) {
  preset = value; // Set globally for display
  /** uint8_t buf[3] = {
    0xB0 | (channel & 0xf),
    commandCode & 0x7f,
    value & 0x7f
  };
  Midi.SendData(buf); **/
  MIDI.sendControlChange(commandCode, value, channel);
  showPreset();
}

void usePresetButtons(bool upPressed, bool downPressed) {
  int prevPreset = preset;
  volatile int newPreset = preset;

  if (upPressed) {
    newPreset = getNewPreset(true);
  }
  if (downPressed) {
    newPreset = getNewPreset(false);
  }

  if ((upPressed || downPressed) && prevPreset != newPreset) {
    changePreset(newPreset);
  }
}

void cycleModes() {
  mode = getNewMode();

  switch (mode) {
    case 1:
      showTap(0);
      break;
    case 0:
      showPreset();
      break;
  }
}

void readButtons() {
  bool upPressed = getValue(BUTTON_UP);
  bool downPressed = getValue(BUTTON_DOWN);

  if (mode == 1) {
    useTapButtons(upPressed, downPressed);
    return;
  }

  if ((millis() - buttonTime) < BUTTON_DEBOUNCE) {
    return;
  }

  if (upPressed && downPressed) {
    cycleModes();
  } else {
    switch (mode) {
      case 1:
        useTapButtons(upPressed, downPressed);
        break;
      case 0:
        usePresetButtons(upPressed, downPressed);
        break;
    }
  }

  buttonTime = millis();
}

void setup() {
#ifdef ADAFRUIT_TRINKET_M0
  // Turn off built-in RED LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  // Turn off built-in Dotstar RGB LED
  strip.begin();
  strip.clear();
  strip.show();
#endif

  MIDI.begin(channel);
  // Serial.begin(115200);
  // usb.Init();

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  display.clear();
  display.setBrightness(3);
  showPreset();

  while( !USBDevice.mounted() ) delay(1);
}

volatile bool presetHasBeenInitialized = false;
void loop() {
  // usb.Task();

  if (!presetHasBeenInitialized) {
    changePreset(0);
    presetHasBeenInitialized = true;
  }

  readButtons();
}
