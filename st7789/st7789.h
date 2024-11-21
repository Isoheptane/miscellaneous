//
// Created by cascade on 11/19/24.
//

#ifndef ST7789_H
#define ST7789_H

#include "stm32f4xx_hal.h"

// Pin Configuration
#define ST7789_CS_GPIO (GPIOD)
#define ST7789_CS_GPIO_PIN (GPIO_PIN_9)

#define ST7789_DC_GPIO (GPIOD)
#define ST7789_DC_GPIO_PIN (GPIO_PIN_8)

#define ST7789_RESET_GPIO (GPIOB)
#define ST7789_RESET_GPIO_PIN (GPIO_PIN_14)

#define ST7789_BL_GPIO (GPIOB)
#define ST7789_BL_GPIO_PIN (GPIO_PIN_12)

// Definitions
#define ST7789_X_OFFSET (52)
#define ST7789_Y_OFFSET (40)

// Functions
void ST7789_Reset(const uint16_t delay);

void ST7789_ChipSelect(const bool selected);

void ST7789_Backlight(const bool on);

void ST7789_Transmit(SPI_HandleTypeDef* hspi, const uint8_t* data, uint16_t size);

void ST7789_Transmit8(SPI_HandleTypeDef* hspi, uint8_t data);

void ST7789_Transmit16(SPI_HandleTypeDef* hspi, uint16_t data);

void ST7789_Command(SPI_HandleTypeDef* hspi, uint8_t command);

void ST7789_Init(SPI_HandleTypeDef* hspi);

void ST7789_DisplayOn(SPI_HandleTypeDef* hspi);

void ST7789_BeginWrite(SPI_HandleTypeDef* hspi, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

#endif //ST7789_H
