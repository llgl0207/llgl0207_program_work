#include "sdcard.h"
#include <SD_MMC.h>

// ESP32-S3 SDIO pins: clk=36, cmd=35, d0=37, d1=38, d2=39, d3=40
// 4-bit mode
bool init_sdcard() {
  SD_MMC.setPins(36, 35, 37, 38, 39, 40);
  if (!SD_MMC.begin("/sdcard", false, false)) {
    Serial.println("SD_MMC mount failed");
    return false;
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return false;
  }
  Serial.printf("SD_MMC mounted. Type %d, Size %llu MB\n", cardType, SD_MMC.cardSize() / (1024ULL * 1024ULL));

  // Quick root listing
  File root = SD_MMC.open("/");
  if (!root) {
    Serial.println("Failed to open root");
    return true; // mounted but listing failed
  }
  File file = root.openNextFile();
  while (file) {
    Serial.printf("%s %s\n", file.isDirectory() ? "<DIR>" : "     ", file.name());
    file = root.openNextFile();
  }
  return true;
}
