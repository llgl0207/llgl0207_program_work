#include <Arduino.h>
#include <SPI.h>
#include "driver/spi_master.h"
#include "driver/gptimer.h"
#include "soc/spi_reg.h"
#include "soc/spi_struct.h"

// 定义 DAC8554 引脚
// LDAC: IO9, CS: IO10, MOSI: IO11, SCLK: IO12
#define DAC_LDAC  9
#define DAC_CS    10
#define DAC_MOSI  11
#define DAC_SCLK  12
#define DAC_MISO  -1 // MISO 不使用，设为 -1 避免与 JOY1_X (IO13) 冲突
#define JOY_A 18
#define JOY_B 21
#define JOY1_X 13
#define JOY1_Y 3
#define JOY2_X 15
#define JOY2_Y 14
#define JOY1_SW 16
#define JOY2_SW 17

// ESP-IDF SPI 相关变量
spi_device_handle_t spi;

// DAC8554 命令寄存器定义（与底层驱动保持一致）
#define DAC8554_BUFFER_WRITE          0x00
#define DAC8554_SINGLE_WRITE          0x10
#define DAC8554_ALL_WRITE             0x20
#define DAC8554_BROADCAST             0x30

/**
 * @brief 使用底层ESP-IDF SPI API发送DAC8554数据帧
 * @param channel DAC通道 (0-3)
 * @param value 16位DAC值 (0-65535)
 * @param writeMode 写入模式 (BUFFER_WRITE, SINGLE_WRITE, ALL_WRITE)
 */
void dac8554_send_frame(uint8_t channel, uint16_t value, uint8_t writeMode) {
    // 配置寄存器格式: [A1 A0 0 C1 C0 0 0 0] + 写入模式
    uint8_t configRegister = (channel << 1) | writeMode;
    
    // 准备SPI传输数据 (24位: 8位配置 + 16位数据)
    uint8_t tx_data[3];
    tx_data[0] = configRegister;
    tx_data[1] = (value >> 8) & 0xFF;  // 数据高8位
    tx_data[2] = value & 0xFF;         // 数据低8位
    
    // 配置SPI传输
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 24;          // 24位数据长度
    t.tx_buffer = tx_data;  // 发送缓冲区
    t.user = (void*)0;      // 用户数据
    
    // 执行SPI传输
    spi_device_polling_transmit(spi, &t);
    
    // 重要：拉低LDAC引脚以更新DAC输出
    digitalWrite(DAC_LDAC, LOW);
    delayMicroseconds(1);   // 保持至少25ns（手册要求）
    digitalWrite(DAC_LDAC, HIGH);
}

/**
 * @brief 初始化ESP-IDF SPI总线
 */
void init_esp_spi() {
    esp_err_t ret;
    
    // SPI总线配置 - 增大DMA传输大小限制
    spi_bus_config_t buscfg = {
        .mosi_io_num = DAC_MOSI,
        .miso_io_num = DAC_MISO,
        .sclk_io_num = DAC_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096  // 增大到4KB，支持大块DMA传输
    };
    
    // 初始化SPI总线 - 启用DMA
    ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        Serial.println("SPI总线初始化失败!");
        return;
    }
    
    // SPI设备配置
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 1,                    // SPI模式1 (CPOL=0, CPHA=1)
        .duty_cycle_pos = 128,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = 20000000,   // 10MHz SPI时钟
        .input_delay_ns = 0,
        .spics_io_num = DAC_CS,       // CS引脚
        .flags = 0,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL
    };
    
    // 添加SPI设备
    ret = spi_bus_add_device(SPI3_HOST, &devcfg, &spi);
    if (ret != ESP_OK) {
        Serial.println("SPI设备添加失败!");
        return;
    }
    
    Serial.println("ESP-IDF SPI初始化完成!");
}
void setup(){

}
void loop(){
    
}