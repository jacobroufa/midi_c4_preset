#ifndef button_h
#define button_h

#include <Arduino.h>

class Button {
  public:
  	Button(int pin);
  	bool isHolding(void);
  	bool isLongPressed(void);
  	bool isPressed(void);
  	void loop(void);
  private:
    int _debounce;
    int _threshold;
    int _longPressThreshold;
    int _pressThreshold;
    int _pin;
    long _time;
    long _holdTime;
    long _lastTap;
    int _state;
    int _prevState;
    bool _isPressed(void);
};

class ButtonFns {
  public:
    ButtonFns(void (*)(), void (*)(), int, Button[]);
    void (* tap)();
    void (* hold)();
    int buttonsLn;
    Button *buttons;
};

class ButtonController {
  public:
    ButtonController(ButtonFns[], int);
    void loop(void);
  private:
    int _debounce;
    long _time;
    int _buttonFnsLn;
    ButtonFns *_buttonFns;
};

#endif
