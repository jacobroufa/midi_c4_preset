#include <Arduino.h>
#include "button.h"

Button::Button(int pin) {
  _debounce = 20;
  _threshold = 1500;
  _longPressThreshold = 400;
  _pressThreshold = 0;
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

ButtonController::ButtonController(ButtonFns buttonFns[], int buttonFnsLn) {
  _buttonFnsLn = buttonFnsLn;
  _buttonFns = buttonFns;
}

void ButtonController::loop(void) {
  for (byte i = 0; i < _buttonFnsLn; i++) {
    ButtonFns *bf = &_buttonFns[i];
    int hold = 0;
    int press = 0;

    for (byte j = 0; j < bf->buttonsLn; j++) {
      Button b = bf->buttons[j];
      b.loop();
      if (b.isLongPressed()) {
        hold += 1;
      } else if (b.isPressed()) {
        press += 1;
      }
    }

    if (hold == bf->buttonsLn) {
      bf->hold();
    } else if (press == bf->buttonsLn) {
      bf->tap();
    }
  }
}
