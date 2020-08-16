#include <TM1637Display.h> // https://github.com/avishorp/TM1637
#include <usbh_midi.h> // https://github.com/gdsports/USB_Host_Library_SAMD

#ifdef ADAFRUIT_TRINKET_M0
// setup Dotstar LED on Trinket M0
#include <Adafruit_DotStar.h> // https://github.com/adafruit/Adafruit_DotStar
#define DATAPIN 7
#define CLOCKPIN 8
Adafruit_DotStar strip = Adafruit_DotStar(1, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
#endif

USBHost usb;
USBH_MIDI Midi(&usb);

const int BUTTON_UP = 0;
const int BUTTON_DOWN = 1;
const int BUTTON_DEBOUNCE = 140; // Any shorter and we get double-presses. Any longer it's sluggish feeling...
const int CHANNEL_DEBOUNCE = 1200; // Show "ch00" on the screen for 1200ms

volatile int channel = 0; // MIDI channel is 0/1 by default
// volatile int commandCode = 103; // Bypass
volatile int commandCode = 104; // Engaged

volatile int preset = 0; // Initial preset value

volatile long buttonTime = 0;
volatile long displayTime = 0;

// instantiate display
const int LCD_DIO = 2;
const int LCD_CLK = 3;
TM1637Display display(LCD_CLK, LCD_DIO);

void changePreset(int value) {
  preset = value; // Set globally for display
  uint8_t buf[3] = {
    0xB0 | (channel & 0xf),
    commandCode & 0x7f,
    value & 0x7f
  };
  Midi.SendData(buf);
}

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

bool getValue(int pin) {
  return digitalRead(pin) == LOW;
}

void showChannel() {
  volatile int tens = 0;
  volatile int ones = channel;

  if (channel >= 10) {
    tens = 1;
    ones = channel - 10;
  }
  
  uint8_t channelSeg[] = {
    SEG_D | SEG_E | SEG_G, // "c"
    SEG_C | SEG_E | SEG_F | SEG_G, // "h"
    display.encodeDigit(tens),
    display.encodeDigit(ones)
  };

  display.setSegments(channelSeg);
  
  delay(CHANNEL_DEBOUNCE);
}

void readButtons() {
  if ((millis() - buttonTime) < BUTTON_DEBOUNCE) {
    return;
  }

  int prevPreset = preset;
  volatile int newPreset = preset;

  bool upPressed = getValue(BUTTON_UP);
  bool downPressed = getValue(BUTTON_DOWN);

  if (upPressed && downPressed) {
    // channel = getNewChannel(); // Uncomment this if we ever desire to change channel?
    showChannel();
  } else {
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

  buttonTime = millis();
}

void updateDisplay() {
  if ((millis() - displayTime) < (BUTTON_DEBOUNCE * 2)) {
    return;
  }

  display.showNumberDec(preset);

  displayTime = millis();
}

void setup() {
  // Turn off built-in RED LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#ifdef ADAFRUIT_TRINKET_M0
  // Turn off built-in Dotstar RGB LED
  strip.begin();
  strip.clear();
  strip.show();
#endif

  usb.Init();

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  display.clear();
  display.setBrightness(3);

  showChannel();

  delay(200);
}

void loop() {
  usb.Task();

  readButtons();
  updateDisplay();
}
