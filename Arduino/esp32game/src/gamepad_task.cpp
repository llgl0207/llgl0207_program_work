#include <Arduino.h>
#include "pins.h"
#include "gamepad_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "draw_esp.h"
#include "ui_menu.h"

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
  // init draw engine with ~10us interval, CPU mode initially
  extern DAC8554 dac; // use instance from main.cpp
  DRAW_InitESP(&dac, 5); // 5us timer interval for higher sample rate
  DRAW_SetModeESP(DRAW_MODE_TIMER_BUFFERED); // buffered output on channels 0/1
  DRAW_SetScaleESP(4, 4); // 12-bit base -> 16-bit via <<4
  dac.broadcastPowerDown(DAC8554_POWERDOWN_NORMAL);
  ui_menu_init();

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

    // process touch input (threshold already handled by TouchButton)
    ui_menu_update(tUp, tDown, tLeft, tRight);
    // Build UI frame for channels 0/1
    DRAW_BeginFrameESP();
    ui_menu_render();
    DRAW_EndFrameESP();

    vTaskDelay(pdMS_TO_TICKS(10)); // UI update pacing
  }
}

void initGamepadTask() {
  if (!s_gamepadTaskHandle) {
    xTaskCreatePinnedToCore(GamepadTask, "GamepadTask", 4096, nullptr, 1, &s_gamepadTaskHandle, 1);
  }
}
