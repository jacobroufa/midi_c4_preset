#include <Arduino.h>
#include "button.h"

Button::Button(int pin) {
  _debounce = 20;
  _threshold = 1500;
  _longPressThreshold = 400;
  _pressThreshold = 60;
  _state = HIGH;
  _prevState = HIGH;
  _pin = pin;
  pinMode(pin, INPUT_PULLUP);
}

bool Button::isHolding(void) {
  return _isPressed() && _holdTime >= _threshold;
}

bool Button::isLongPressed(void) {
  return _isPressed() && _holdTime >= _longPressThreshold && _holdTime < _threshold;
}

bool Button::isPressed(void) {
  return _isPressed() && _holdTime >= _pressThreshold;
}

void Button::loop(void) {
  const long now = millis();

  if ((now - _time) < _debounce) return;

  _state = digitalRead(_pin);
  _time = now;

  if (_prevState != _state) {
    _prevState = _state;

    if (_isPressed()) {
      _holdTime = 0;
      _lastTap = now;
    }
  } else {
    if (_isPressed()) {
      _holdTime = now - _lastTap;
    }
  }
}

bool Button::_isPressed(void) {
  return _state == LOW;
}

ButtonFns::ButtonFns(void (*_tap)(), void (*_hold)(), int _buttonsLn, Button _buttons[]) {
  tap = _tap;
  hold = _hold;
  buttonsLn = _buttonsLn;
  buttons = _buttons;
}

ButtonController::ButtonController(ButtonFns buttonFns[], int buttonFnsLn, ButtonFns twoButtonFns[], int twoButtonFnsLn) {
  _buttonFnsLn = buttonFnsLn;
  _buttonFns = buttonFns;
  _twoButtonFnsLn = twoButtonFnsLn;
  _twoButtonFns = twoButtonFns;
  _debounce = 40;
}

void ButtonController::loop(void) {
  const long now = millis();

  if ((now - _time) < _debounce) return;

  _time = now;

  volatile bool _dualPress = false;

  for (byte i = 0; i < _twoButtonFnsLn; i++) {
    ButtonFns *bf = &_twoButtonFns[i];
    int press = 0;

    for (byte j = 0; j < bf->buttonsLn; j++) {
      if (bf->buttons[j].isPressed()) {
        press += 1;
      }
    }

    if (press == bf->buttonsLn) {
      _dualPress = true;
      bf->tap();
    }
  }

  if (!_dualPress) {
    for (byte i = 0; i < _buttonFnsLn; i++) {
      ButtonFns *bf = &_buttonFns[i];

      if (bf->buttonsLn == 1 && bf->buttons[0].isPressed()) {
        bf->tap();
      }
    }
  }
}
