# MIDI Preset Changer

Intended for use with the [Source Audio C4 Synth](https://www.sourceaudio.net/c4_synth.html) but will maybe work with other One Series pedals.

---

## Requirements

### Hardware

- Adafruit Trinket M0
- TM1637 7-segment display
- 2x momentary buttons
- 1590A case (or similar)

### Software Dependencies

- https://github.com/avishorp/TM1637
- https://github.com/gdsports/USB_Host_Library_SAMD

---

## Pin Assignments

- 0 - Up Button
- 1 - Down Button
- 2 - LCD I/O
- 3 - LCD Clock

---

Prompted by [a discussion at The Gear Page](https://www.thegearpage.net/board/index.php?threads/source-audio-c4-homebrew-arduino-midi-controller.2078129). Thanks to user morrissey007 for the motivation. :)

## Functionality

Button 0 press will increment preset (default/startup = 0), Button 1 will decrement. Pressing both buttons simultaneously will display the channel (set to 0 in code). Uncomment L114 in order to allow this to be changed. Don't know if this is useful -- I may have the combined press do something different in the future.
