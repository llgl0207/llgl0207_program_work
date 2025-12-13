#include <Arduino.h>
#include <SPI.h>
#include "DAC8554.h"
#include "pins.h"
#include "gamepad_task.h"

// 引脚定义移至 pins.h

// 实例化 DAC 对象
// 参数: CS引脚, SPI对象
DAC8554 dac(DAC_CS, &SPI);

// 手柄任务在独立文件中实现

void setup() {
  // 初始化串口
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP32-S3 Gamepad Test");
  
  Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));

  // 初始化手柄按键 (使用内部上拉，按下为 LOW)
  pinMode(JOY1_SW, INPUT_PULLUP);
  pinMode(JOY2_SW, INPUT_PULLUP);
  pinMode(JOY_A, INPUT_PULLDOWN);
  pinMode(JOY_B, INPUT_PULLDOWN);

  // 配置 ADC 分辨率 (默认12位: 0-4095)
  analogReadResolution(12);

  // 1. 初始化 LDAC 引脚
  // LDAC 为低电平时，DAC 输出会立即更新
  // 如果需要同步更新多个 DAC，可以先拉高，写入数据后，再拉低
  pinMode(DAC_LDAC, OUTPUT);
  digitalWrite(DAC_LDAC, LOW); 

  // 2. 初始化 SPI 总线
  // 参数: SCK, MISO, MOSI, SS
  // 注意：必须在 dac.begin() 之前调用
  SPI.begin(DAC_SCLK, DAC_MISO, DAC_MOSI, DAC_CS);

  // 3. 初始化 DAC 库
  dac.begin();
  dac.setSPIspeed(1000000); // 降低 SPI 速度到 1MHz 以提高稳定性

  // 关闭 DAC8554 输出（高阻态）
  dac.broadcastPowerDown(DAC8554_POWERDOWN_HIGH_IMP);

  // 启动手柄/按键检测 FreeRTOS 任务
  initGamepadTask();

  Serial.println("Starting Touch Pad Test with Debounce...");
}

void loop() {
  // 空置：手柄与按键检测已移入 FreeRTOS 任务
}