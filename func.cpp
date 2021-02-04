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

// from SO
void eepromRead(long addr, void* output, long length) {
    byte* src; 
    byte* dst;
    src = (byte*)addr;
    dst = (byte*)output;
    for (long i = 0; i < length; i++) {
        *dst++ = eeprom_read_byte(src++);
    }
}

void eepromWrite(long addr, void* input, long length) {
    byte* src; 
    byte* dst;
    src = (byte*)input;
    dst = (byte*)addr;
    for (long i = 0; i < length; i++) {
        eeprom_write_byte(dst++, *src++);
    }
}
uint16_t currentAddress;
struct {
    uint16_t x;
    uint16_t y;
} data;

struct {

} output;
uint16_t input

eepromWrite(currentAddress, data, sizeof(data);
eepromRead(currentAddress, output, sizeof(data));

// from Rory:
#define SETTINGS_START_ADDR 1
void save_settings(device_settings *settings)
{
    unsigned char *settingsPtr = settings;
    u8 settingsSize = sizeof(device_settings);
    u8 i;
    u8 address = SETTINGS_START_ADDR;
    for (i = 0; i < settingsSize; i++, address++)
    {
        ee_write(address, *settingsPtr);
    }
}
void read_settings(device_settings *settings)
{
    unsigned char *settingsPtr = settings;
    u8 settingsSize = sizeof(device_settings);
    u8 address = SETTINGS_START_ADDR;
    u8 i;
    for (i = 0; i < settingsSize; i++, address++)
    {
        *settingsPtr = ee_read(address);
    }
}

*/
