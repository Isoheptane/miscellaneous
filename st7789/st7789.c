//
// Created by cascade on 11/19/24.
//

#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "st7789.h"

void ST7789_Reset(const uint16_t delay)
{
    HAL_GPIO_WritePin(ST7789_RESET_GPIO, ST7789_RESET_GPIO_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(ST7789_RESET_GPIO, ST7789_RESET_GPIO_PIN, GPIO_PIN_SET);
    HAL_Delay(delay);
}

void ST7789_ChipSelect(const bool selected)
{
    HAL_GPIO_WritePin(ST7789_CS_GPIO, ST7789_CS_GPIO_PIN, selected ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void ST7789_Backlight(const bool on)
{
    HAL_GPIO_WritePin(ST7789_BL_GPIO, ST7789_BL_GPIO_PIN, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void ST7789_Transmit(SPI_HandleTypeDef* hspi, const uint8_t* data, uint16_t size)
{
    HAL_SPI_Transmit(hspi, data, size, 1000);
}

void ST7789_Transmit8(SPI_HandleTypeDef* hspi, uint8_t data)
{
    HAL_SPI_Transmit(hspi, &data, 1, 1000);
}

void ST7789_Transmit16(SPI_HandleTypeDef* hspi, uint16_t data)
{
    HAL_SPI_Transmit(hspi, (uint8_t*)&data, 2, 1000);
}

void ST7789_Command(SPI_HandleTypeDef* hspi, uint8_t command)
{
    HAL_GPIO_WritePin(ST7789_DC_GPIO, ST7789_DC_GPIO_PIN, GPIO_PIN_RESET);
    ST7789_Transmit8(hspi, command);
    HAL_GPIO_WritePin(ST7789_DC_GPIO, ST7789_DC_GPIO_PIN, GPIO_PIN_SET);
}

const static uint8_t GAMMA_POS_SEQ[] = {0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
const static uint8_t GAMMA_NEG_SEQ[] = {0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};

void ST7789_Init(SPI_HandleTypeDef* hspi)
{
    ST7789_Reset(150);

    ST7789_ChipSelect(true);
    // Memory Data Access Control
    ST7789_Command(hspi, 0x36);
    ST7789_Transmit8(hspi, 0b01100000);

    // Pixel Format
    ST7789_Command(hspi, 0x3A);
    ST7789_Transmit8(hspi, 0b00000101); // RGB565

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

void ST7789_DisplayOn(SPI_HandleTypeDef* hspi)
{
    ST7789_Command(hspi, 0x29);
}

void ST7789_BeginWrite(SPI_HandleTypeDef* hspi, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t x1 = x + ST7789_Y_OFFSET;
    uint16_t x2 = x1 + w - 1;
    uint16_t y1 = y + ST7789_X_OFFSET + 1;
    uint16_t y2 = y1 + h - 1;

    ST7789_Command(hspi, 0x2A); // Column Address Set
    ST7789_Transmit8(hspi, (x1>>8)&0xff);
    ST7789_Transmit8(hspi, x1&0xff);
    ST7789_Transmit8(hspi, (x2>>8)&0xff);
    ST7789_Transmit8(hspi, x2&0xff);

    ST7789_Command(hspi, 0x2B); // Row Address Set
    ST7789_Transmit8(hspi, (y1>>8)&0xff);
    ST7789_Transmit8(hspi, y1&0xff);
    ST7789_Transmit8(hspi, (y2>>8)&0xff);
    ST7789_Transmit8(hspi, y2&0xff);

    ST7789_Command(hspi, 0x2C); // Memory Write
}

/*
void tdd_drv_init()
{
    tdd_if_reset();

    tdd_if_bus_enable();

    tdd_if_command(0x36); // Memory Data Access Control
    tdd_if_data((MY<<7)|(MX<<6)|(MV<<5)|(TDD_CFG_RGB_REVERSED<<3)); // MY MX MV ML RGB MH _ _

    tdd_if_command(0x3A); // Interface Pixel Format
    tdd_if_data(0x05); // RGB565

    tdd_if_command(0xB2); // Frame Rate Control
    tdd_if_data(0x0C);
    tdd_if_data(0x0C);
    tdd_if_data(0x00);
    tdd_if_data(0x33);
    tdd_if_data(0x33);

    tdd_if_command(0xC0); // Power Control 1
    tdd_if_data(0x2C);

    tdd_if_command(0xC2); // Power Control 3
    tdd_if_data(0x01);

    tdd_if_command(0xC3); // Power Control 4
    tdd_if_data(0x12);

    tdd_if_command(0xC4); // Power Control 5
    tdd_if_data(0x20);

    tdd_if_command(0xE0); // Gamma (+polarity) Correction
    tdd_if_data_buffer((uint8_t *)gamma_positive, GAMMA_SEQUENCE_LENGTH);

    tdd_if_command(0xE1); // Gamma (-polarity) Correction
    tdd_if_data_buffer((uint8_t *)gamma_negative, GAMMA_SEQUENCE_LENGTH);

#if TDD_CFG_COLOR_INVERTED
    tdd_if_command(0x21); // Display Inversion On
#endif

    tdd_if_command(0x11); // Sleep Out

}

void tdd_drv_display_on()
{
    tdd_if_command(0x29); // Display On
}

void tdd_drv_set_window(uint16_t x,uint16_t y,uint16_t w,uint16_t h)
{
    uint16_t x1 = x + X_OFFSET;
    uint16_t x2 = x1 + w - 1;
    uint16_t y1 = y + Y_OFFSET;
    uint16_t y2 = y1 + h - 1;

    tdd_if_command(0x2a); // Column Address Set
    tdd_if_data((x1>>8)&0xff);
    tdd_if_data(x1&0xff);
    tdd_if_data((x2>>8)&0xff);
    tdd_if_data(x2&0xff);

    tdd_if_command(0x2b); // Row Address Set
    tdd_if_data((y1>>8)&0xff);
    tdd_if_data(y1&0xff);
    tdd_if_data((y2>>8)&0xff);
    tdd_if_data(y2&0xff);
}

void tdd_drv_start_memory_write()
{
    tdd_if_command(0x2c); // Memory Write
}
*/