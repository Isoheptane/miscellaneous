#pragma once

/*  =============================
 *      ST7789 Configuration
 *  ============================= */

/* Reset Pin */
#define ST7789_RESET_GPIO (GPIOB)
#define ST7789_RESET_PIN (GPIO_PIN_0)

/* D/C Pin */
#define ST7789_DC_GPIO (GPIOB)
#define ST7789_DC_PIN (GPIO_PIN_1)

/* Chip Select */
#define ST7789_CS_GPIO (GPIOA)
#define ST7789_CS_PIN (GPIO_PIN_4)

/* Backlight Control */
#define ST7789_BL_GPIO (GPIOA)
#define ST7789_BL_PIN (GPIO_PIN_3)
#define ST7789_BL_LOW_ACTIVE

/* Display Area Setting */
#define ST7789_MEM_X_OFFSET (53)
#define ST7789_MEM_X_SIZE (135)
#define ST7789_MEM_Y_OFFSET (40)
#define ST7789_MEM_Y_SIZE (240)

/* Data Direction Setting */
// For display data direction, refer to
// https://www.waveshare.com/w/upload/a/ae/ST7789_Datasheet.pdf#page=125
#define ST7789_EXCHANGE_XY (1)
#define ST7789_MIRROR_X (1)
#define ST7789_MIRROR_Y (0)

/* Color Format Setting */
#define ST7789_PIXEL_FORMAT_12BIT (0b011)
#define ST7789_PIXEL_FORMAT_16BIT (0b101)
#define ST7789_PIXEL_FORMAT_18BIT (0b110)
#define ST7789_PIXEL_FORMAT (ST7789_PIXEL_FORMAT_18BIT)

/* SPI Interface Setting */
#define ST7789_SPI_TIMEOUT (1000)

/*  =============================
 *      ST7789 Implementation
 *  ============================= */

#include <stdbool.h>
#include "stm32f4xx_hal.h"

// Functions

/// Select ST7789 on SPI bus
static inline void ST7789_ChipSelect(const bool selected) {
    // Chip Select is low active
    HAL_GPIO_WritePin(ST7789_CS_GPIO, ST7789_CS_PIN, !selected);
}

/// Reset ST7789
/// disable_delay: delay after resetting ST7789.
/// enable_delay : delay after enabling ST7789.
/// Recommended values are disable_delay = 20ms and enable_delay = 150ms.
static inline void ST7789_Reset(const uint16_t disable_delay, const uint16_t enable_delay) {
    // Reset is low active
    ST7789_ChipSelect(false);
    HAL_GPIO_WritePin(ST7789_RESET_GPIO, ST7789_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(disable_delay);
    HAL_GPIO_WritePin(ST7789_RESET_GPIO, ST7789_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(enable_delay);
}

/// LED backlight control
static inline void ST7789_SetBacklight(const bool on) {
#ifndef ST7789_BL_LOW_ACTIVE
    HAL_GPIO_WritePin(ST7789_BL_GPIO, ST7789_BL_PIN, on);
#else
    HAL_GPIO_WritePin(ST7789_BL_GPIO, ST7789_BL_PIN, !on);
#endif
}

/// Transmit data to ST7789
static inline HAL_StatusTypeDef ST7789_Transmit(SPI_HandleTypeDef* hspi, const uint8_t* data, uint16_t size, uint16_t timeout) {
    ST7789_ChipSelect(true);
    return HAL_SPI_Transmit(hspi, data, size, timeout);
    ST7789_ChipSelect(false);
}

/// Transmit a 8 bit data to ST7789
static inline HAL_StatusTypeDef ST7789_Transmit8(SPI_HandleTypeDef* hspi, uint8_t data) {
    return ST7789_Transmit(hspi, &data, 1, ST7789_SPI_TIMEOUT);
}

/// Transmit a 16 bit data to ST7789
static inline HAL_StatusTypeDef ST7789_Transmit16(SPI_HandleTypeDef* hspi, uint16_t data) {
    return ST7789_Transmit(hspi, (uint8_t*)&data, 2, ST7789_SPI_TIMEOUT);
}

/// Transmit a 24 bit data to ST7789
static inline HAL_StatusTypeDef ST7789_Transmit24(SPI_HandleTypeDef* hspi, uint32_t data) {
    return ST7789_Transmit(hspi, (uint8_t*)&data, 3, ST7789_SPI_TIMEOUT);
}

/// Send a command to ST7789
/// If argument is required, send arguments using ST7789_Transmit functions
static inline void ST7789_Command(SPI_HandleTypeDef* hspi, uint8_t command) {
    HAL_GPIO_WritePin(ST7789_DC_GPIO, ST7789_DC_PIN, GPIO_PIN_RESET);
    ST7789_Transmit8(hspi, command);
    HAL_GPIO_WritePin(ST7789_DC_GPIO, ST7789_DC_PIN, GPIO_PIN_SET);
}

const static uint8_t GAMMA_POS_SEQ[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F,
                                        0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
const static uint8_t GAMMA_NEG_SEQ[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F,
                                        0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};

/// Initialize ST7789
/// Notice: It is required to call ST7789_Reset() before ST7789 init
static inline void ST7789_Init(SPI_HandleTypeDef* hspi) {

#ifndef ST7789_HARDWARE_NSS
    ST7789_ChipSelect(true);
#endif

    // Memory Data Access Control
    ST7789_Command(hspi, 0x36);
    ST7789_Transmit8(hspi, ST7789_EXCHANGE_XY << 5 | ST7789_MIRROR_X << 6 | ST7789_MIRROR_Y << 7);

    // Pixel Format
    ST7789_Command(hspi, 0x3A);
    ST7789_Transmit8(hspi, ST7789_PIXEL_FORMAT);

    // Porch Setting
    ST7789_Command(hspi, 0xB2);
    ST7789_Transmit8(hspi, 0x0C);
    ST7789_Transmit8(hspi, 0x0C);
    ST7789_Transmit8(hspi, 0x00);
    ST7789_Transmit8(hspi, 0x33);
    ST7789_Transmit8(hspi, 0x33);

    // Gate Control
    ST7789_Command(hspi, 0xB7);
    ST7789_Transmit8(hspi, 0x35);

    // VCOMS Control
    ST7789_Command(hspi, 0xBB);
    ST7789_Transmit8(hspi, 0x19);

    // LCM Control
    ST7789_Command(hspi, 0xC0);
    ST7789_Transmit8(hspi, 0x2C);

    // Power - VDV/VRH Enable Type
    ST7789_Command(hspi, 0xC2);
    ST7789_Transmit8(hspi, 0x01); // VDV/VRH from command
    ST7789_Transmit8(hspi, 0xFF);

    // Power - VRH Set
    ST7789_Command(hspi, 0xC3);
    ST7789_Transmit8(hspi, 0x12);

    // Power - VDV Set
    ST7789_Command(hspi, 0xC4);
    ST7789_Transmit8(hspi, 0x20);

    // Frame Rate Setting
    ST7789_Command(hspi, 0xC6);
    ST7789_Transmit8(hspi, 0x0F);

    // Power Control 1
    ST7789_Command(hspi, 0xD0);
    ST7789_Transmit8(hspi, 0xA4);
    ST7789_Transmit8(hspi, 0xA1);

    // Gamma Correction
    ST7789_Command(hspi, 0xE0);
    ST7789_Transmit(hspi, GAMMA_POS_SEQ, sizeof(GAMMA_POS_SEQ));
    ST7789_Command(hspi, 0xE1);
    ST7789_Transmit(hspi, GAMMA_NEG_SEQ, sizeof(GAMMA_NEG_SEQ));

    // Inverse Mode On
    ST7789_Command(hspi, 0x21);

    // Exit From Sleep
    ST7789_Command(hspi, 0x11);
}

/// Enable ST7789 Display
static inline void ST7789_SetDisplay(SPI_HandleTypeDef* hspi, const bool on) {
    ST7789_Command(hspi, on ? 0x29 : 0x28);
}

/// Set write area for ST7789 and begin writing data
/// Coordinates are synced with display data direction settings
/// For display data direction, refer to https://www.waveshare.com/w/upload/a/ae/ST7789_Datasheet.pdf#page=125
static inline void ST7789_SetWriteArea(SPI_HandleTypeDef* hspi, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint16_t xs, xe, ys, ye;
    if (!ST7789_EXCHANGE_XY) {
        xs = !ST7789_MIRROR_X ?
            (ST7789_MEM_X_OFFSET + x) :
            (ST7789_MEM_X_OFFSET + ST7789_MEM_X_SIZE - x - w);
        xe = !ST7789_MIRROR_X ?
            (ST7789_MEM_X_OFFSET + x + w - 1) :
            (ST7789_MEM_X_OFFSET + ST7789_MEM_X_SIZE - x - 1);
        ys = !ST7789_MIRROR_Y ?
            (ST7789_MEM_Y_OFFSET + y) :
            (ST7789_MEM_Y_OFFSET + ST7789_MEM_Y_SIZE - y - h);
        ye = !ST7789_MIRROR_Y ?
            (ST7789_MEM_Y_OFFSET + y + h - 1) :
            (ST7789_MEM_Y_OFFSET + ST7789_MEM_Y_SIZE - y - 1);
    } else {
        xs = !ST7789_MIRROR_Y ?
            (ST7789_MEM_Y_OFFSET + x) :
            (ST7789_MEM_Y_OFFSET + ST7789_MEM_Y_SIZE - x - w);
        xe = !ST7789_MIRROR_Y ?
            (ST7789_MEM_Y_OFFSET + x + w - 1) :
            (ST7789_MEM_Y_OFFSET + ST7789_MEM_Y_SIZE - x - 1);
        ys = !ST7789_MIRROR_X ?
            (ST7789_MEM_X_OFFSET + ST7789_MEM_X_SIZE - y - h) :
            (ST7789_MEM_X_OFFSET + y);
        ye = !ST7789_MIRROR_X ?
            (ST7789_MEM_X_OFFSET + ST7789_MEM_X_SIZE - y - 1) :
            (ST7789_MEM_X_OFFSET + y + h - 1);
    }

    // big endian transfer is required
    ST7789_Command(hspi, 0x2A); // Column Address Set
    ST7789_Transmit8(hspi, (xs >> 8) & 0xff);
    ST7789_Transmit8(hspi, xs & 0xff);
    ST7789_Transmit8(hspi, (xe >> 8) & 0xff);
    ST7789_Transmit8(hspi, xe & 0xff);

    ST7789_Command(hspi, 0x2B); // Row Address Set
    ST7789_Transmit8(hspi, (ys >> 8) & 0xff);
    ST7789_Transmit8(hspi, ys & 0xff);
    ST7789_Transmit8(hspi, (ye >> 8) & 0xff);
    ST7789_Transmit8(hspi, ye & 0xff);

    ST7789_Command(hspi, 0x2C); // Memory Write
}
