#include <Arduino.h>
#include "pins.h"
#include "gamepad_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class TouchButton {
  private:
    uint8_t pin;
    uint32_t threshold;
    bool lastState;
    bool lastReading;
    uint32_t lastDebounceTime;
    uint32_t debounceDelay;

  public:
    TouchButton(uint8_t p, uint32_t t = 40000, uint32_t d = 50)
      : pin(p), threshold(t), lastState(false), lastReading(false),
        lastDebounceTime(0), debounceDelay(d) {}

    void update() {
      int reading = touchRead(pin);
      bool isTouched = (reading > threshold);
      if (isTouched != lastReading) {
        lastDebounceTime = millis();
      }
      lastReading = isTouched;
      if ((millis() - lastDebounceTime) > debounceDelay) {
        if (isTouched != lastState) {
          lastState = isTouched;
        }
      }
    }

    bool getState() { return lastState; }
};

static TaskHandle_t s_gamepadTaskHandle = nullptr;
static TouchButton btn4(4, 30000);
static TouchButton btn5(5, 30000);
static TouchButton btn6(6, 30000);
static TouchButton btn7(7, 30000);

static void GamepadTask(void* pv) {
  for (;;) {
    // Update touch buttons
    btn4.update();
    btn5.update();
    btn6.update();
    btn7.update();

    // Read joysticks and buttons
    int j1x = 4095 - analogRead(JOY1_X);
    int j1y = analogRead(JOY1_Y);
    int j1sw = digitalRead(JOY1_SW);

    int j2x = 4095 - analogRead(JOY2_X);
    int j2y = analogRead(JOY2_Y);
    int j2sw = digitalRead(JOY2_SW);

    int btnA = digitalRead(JOY_A);
    int btnB = digitalRead(JOY_B);

    int tUp = btn4.getState();
    int tLeft = btn5.getState();
    int tDown = btn6.getState();
    int tRight = btn7.getState();

    Serial.printf("J1:%4d,%4d,%d | J2:%4d,%4d,%d | A:%d B:%d | Dpad:%d,%d,%d,%d\n",
                  j1x, j1y, j1sw,
                  j2x, j2y, j2sw,
                  btnA, btnB,
                  tUp, tLeft, tDown, tRight);

    vTaskDelay(pdMS_TO_TICKS(10)); // ~100 Hz
  }
}

void initGamepadTask() {
  if (!s_gamepadTaskHandle) {
    xTaskCreatePinnedToCore(GamepadTask, "GamepadTask", 4096, nullptr, 1, &s_gamepadTaskHandle, 1);
  }
}
