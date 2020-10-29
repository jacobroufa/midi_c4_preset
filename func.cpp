// TODO: try to use this, or figure out other usb host midi
// #include <usbh_midi.h> // https://github.com/gdsports/USB_Host_Library_SAMD
// USBHost usb;
// USBH_MIDI Midi(&usb);
/*
volatile int channel = 0; // MIDI channel is 0/1 by default
volatile int preset = 0; // Initial preset value
volatile int commandCode = 103; // Bypass
// volatile int commandCode = 104; // Engaged

void changePreset(int value) {
  preset = value; // Set globally for display
  uint8_t buf[3] = {
	0xB0 | (channel & 0xf),
	commandCode & 0x7f,
	value & 0x7f
  };
  // Midi.SendData(buf);
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

int getNewPreset(bool up) {
  int max = 127;
  return getNewValue(up, max, preset);
}

int getNewChannel(bool up) {
  int max = 15;
  return getNewValue(up, max, channel);
}
*/
