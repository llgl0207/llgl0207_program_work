#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"

// 1-bit 模式配置
// 用户指定：CMD=38, CLK=39, D0=40
// 这些引脚 (38, 39, 40) 不与 OPI PSRAM (33-37) 冲突，是安全的。

const int SD_MMC_CMD = 38;
const int SD_MMC_CLK = 39;
const int SD_MMC_D0 = 40;

// D1, D2, D3 在 1-bit 模式下不使用数据传输，但 setPins 需要参数。
// 我们指定为任意未使用的安全引脚，以免驱动程序误操作其他重要引脚。
const int SD_MMC_D1 = 14; 
const int SD_MMC_D2 = 15; 
const int SD_MMC_D3 = 16; 

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    Serial.println("\nStarting SD Card Test on ESP32-S3 SDIO (1-bit Mode)...");

    if(psramFound()){
        Serial.printf("PSRAM Found! Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
        Serial.println("PSRAM is using IO: 26-32 and 33-37");
    } else {
        Serial.println("PSRAM Not Found");
    }

    // 设置SDIO引脚
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0, SD_MMC_D1, SD_MMC_D2, SD_MMC_D3);

    // 初始化SD卡
    // "/sdcard" 是挂载点
    // true 表示使用 1-bit 模式 (只使用 CMD, CLK, D0)
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("Card Mount Failed");
        Serial.println("Check if the pins are correct and the card is inserted.");
        return;
    }

    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    // 严格区分：如果识别为MMC卡则提示不符合要求
    if (cardType == CARD_MMC) {
        Serial.println("Warning: MMC card detected! This program expects an SD card (SD/SDHC/SDXC).");
        // 如果您希望在检测到MMC时停止，可以在这里return
        // return; 
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
    
    Serial.println("SD Card initialized successfully using SDIO 1-bit mode.");
}

void loop() {
    // Nothing to do here
    delay(1000);
}
