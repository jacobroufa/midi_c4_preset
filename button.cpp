#include <vector>
#include <Arduino.h>
#include "button.h"

Button::Button(int pin) {
  _debounce = 20;
  _threshold = 1500;
  _longPressThreshold = 400;
  _pressThreshold = 50;
  _pin = pin;
  pinMode(pin, INPUT_PULLUP);
}

bool Button::isHolding() {
  return _isPressed() && _holdTime >= _threshold;
}

bool Button::isLongPressed() {
  return _isPressed() && _holdTime >= _longPressThreshold && _holdTime < _threshold;
}

bool Button::isPressed() {
  return _isPressed() && _holdTime >= _pressThreshold;
}

void Button::loop() {
  const long now = millis();

  if ((now - _time) < _debounce) {
  return;
  }

  _state = digitalRead(_pin);
  _time = now;

  if (_isPressed()) {
    if (_prevState != _state) {
    _lastTap = now;
    } else {
    _holdTime = now - _lastTap;
    }
  }
}

bool Button::_isPressed() {
  return _state == LOW;
}

ButtonController::ButtonController(std::vector<ButtonFns> buttonFns) {
  _debounce = 20;
  _buttonFns = buttonFns;
}

void ButtonController::loop() {
  const long now = millis();

  if ((now - _time) < _debounce) {
    return;
  }

  _time = now;

  for (ButtonFns bf : _buttonFns) {
    int numButtons = sizeof(bf.buttons)/sizeof(bf.buttons[0]);
    int hold = 0;
    int press = 0;

    for (Button b : bf.buttons) {
      b.loop();
      if (b.isHolding())
        hold += 1;
      if (b.isPressed())
        press += 1;
    }

    if (hold == numButtons) {
      bf.hold();
    } else if (press == numButtons) {
      bf.tap();
    }
  }
}
