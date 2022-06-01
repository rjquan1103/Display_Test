#include "fonts.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include <stdio.h>
#include "stm32f4xx_hal.h"

typedef enum {
	SH1107_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
	SH1107_COLOR_WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
} SH1107_COLOR_t;

/* SH1107 settings */
/* SH1107 width in pixels */
#ifndef SH1107_WIDTH
#define SH1107WIDTH            128
#endif
/* SSD1306 LCD height in pixels */
#ifndef SH1107HEIGHT
#define SH1107HEIGHT           64
#endif

// I2C Address of OLED Display
#ifndef OLED_I2C
#define OLED_I2C 0x3c
#endif

/**
 * @brief  Initializes SH1107 LCD
 * @param  None
 * @retval Initialization status:
 *           - 0: LCD was not detected on I2C port
 *           - > 0: LCD initialized OK and ready to use
 */
uint8_t DisplayInit(void);

/**
 * @brief  Updates buffer from internal RAM to LCD
 * @note   This function must be called each time you do some changes to LCD, to update buffer from RAM to LCD
 * @param  None
 * @retval None
 */
void SH1107_UpdateScreen(void);

/**
 * @brief  Fills entire LCD with desired color
 * @note   @ref SH1107_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  Color: Color to be used for screen fill. This parameter can be a value of @ref SH1107_COLOR_t enumeration
 * @retval None
 */
void SH1107_Fill(SH1107_COLOR_t color);

/**
 * @brief  Draws pixel at desired location
 * @note   @ref SH1107_UpdateScreen() must called after that in order to see updated LCD screen
 * @param  x: X location. This parameter can be a value between 0 and SH1107_WIDTH - 1
 * @param  y: Y location. This parameter can be a value between 0 and SH1107_HEIGHT - 1
 * @param  color: Color to be used for screen fill. This parameter can be a value of @ref SH1107_COLOR_t enumeration
 * @retval None
 */
void SH1107_DrawPixel(uint16_t x, uint16_t y, SH1107_COLOR_t color);

/**
 * @brief  Puts character to internal RAM
 * @note   @ref SH1107_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  ch: Character to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color: Color used for drawing. This parameter can be a value of @ref SH1107_COLOR_t enumeration
 * @retval Character written
 */
char SH1107_Putc(char ch, FontDef_t* Font, SH1107_COLOR_t color);


/**
 * @brief  Puts string to internal RAM
 * @note   @ref SH1107_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  *str: String to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color: Color used for drawing. This parameter can be a value of @ref SH1107_COLOR_t enumeration
 * @retval Zero on success or character value when function failed
 */
char SH1107_Puts(char* str, FontDef_t* Font, SH1107_COLOR_t color);

/*
 * @brief Display Blank Screen
 */
void SH1107_Clear (void);

/**
 * @brief Initializes I2C Master
 * @retval = 0 If I2C not detected
 * @retval > 0 if all good
 */

void SH1107_I2C_Init();

/**
 * @brief  Writes single byte to slave
 * @param  *I2Cx: I2C used
 * @param  address: 7 bit slave address, left aligned, bits 7:1 are used, LSB bit is not used
 * @param  reg: register to write to
 * @param  data: data to be written
 * @retval None
 */


void SH1107_I2C_Write(uint8_t address, uint8_t reg, uint8_t data);

/**
 * @brief  Writes multi bytes to slave
 * @param  *I2Cx: I2C used
 * @param  address: 7 bit slave address, left aligned, bits 7:1 are used, LSB bit is not used
 * @param  reg: register to write to
 * @param  *data: pointer to data array to write it to slave
 * @param  count: how many bytes will be written
 * @retval None
 */
void SH1107_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t *data, uint16_t count);

