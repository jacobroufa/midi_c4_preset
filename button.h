#ifndef button_h
#define button_h

#include <vector>
#include <Arduino.h>

class Button {
  public:
  	Button(int pin);
  	bool isHolding();
  	bool isLongPressed();
  	bool isPressed();
  	void loop();
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
    bool _isPressed();
};

typedef struct ButtonFns {
  void (* tap)();
  void (* hold)();
  std::vector<Button> buttons;
} ButtonFns;

class ButtonController {
  public:
  	ButtonController(std::vector<ButtonFns> buttonFns);
  	void loop();
  private:
    int _debounce;
    long _time;
    std::vector<ButtonFns> _buttonFns;
};

#endif
