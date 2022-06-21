#include "Display.h"

extern I2C_HandleTypeDef hi2c1;

#define SH1107_X_OFFSET_LOWER (0x60 & 0x0F)
#define SH1107_X_OFFSET_UPPER ((0x60 >> 4) & 0x07)

static uint8_t SH1107_Buffer[SH1107_BUFFER_SIZE];

// Private struct for Sh1107 object
typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Initialized;
	uint8_t Inverted;
	uint8_t DisplayOn;
} SH1107_t;

// Private variable
static SH1107_t SH1107;

// Send a byte to the command register
void SH1107_WRITECOMMAND(uint8_t byte) {
	HAL_I2C_Mem_Write(&hi2c1, OLED_I2C, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
}

// Send data
void SH1107_WriteData(uint8_t *buffer, size_t buff_size) {
	HAL_I2C_Mem_Write(&hi2c1, OLED_I2C, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}

void SH1107_SetDisplayOn(const uint8_t on) {
    uint8_t value;
    if (on) {
        value = 0xAF;   // Display on
        SH1107.DisplayOn = 1;
    } else {
        value = 0xAE;   // Display off
        SH1107.DisplayOn = 0;
    }
    SH1107_WRITECOMMAND(value);
}

void SH1107_SetContrast(const uint8_t value) {
    const uint8_t kSetContrastControlRegister = 0x81;
    SH1107_WRITECOMMAND(kSetContrastControlRegister);
    SH1107_WRITECOMMAND(value);
}

// Position the cursor
void SH1107_SetCursor(uint8_t x, uint8_t y) {
    SH1107.CurrentX = x;
    SH1107.CurrentY = y;
}

// Constructor for the Adafruit FeatherWing 128x64 OLED
// Initialize Display
uint8_t DisplayInit(void) {

	HAL_Delay(100);
	if(HAL_I2C_IsDeviceReady(&hi2c1, OLED_I2C, 1, 20000) != HAL_OK) {
		return(0);
	}


	// SH1107 128x64 has 128 columns and 8 rows/pages. Each row containing 8 bytes ~ 64 total
	SH1107_SetDisplayOn(0);					// Display Off

	SH1107_WRITECOMMAND(0x00);				// Set lower column address
	SH1107_WRITECOMMAND(0x00);

	SH1107_WRITECOMMAND(0x10);				// Set upper column address
	SH1107_WRITECOMMAND(0x40);

	SH1107_WRITECOMMAND(0xB0);				// Set Page address
	SH1107_WRITECOMMAND(0x00);

	SH1107_WRITECOMMAND(0xDC);				// Set display start line
	SH1107_WRITECOMMAND(0x00);

	SH1107_WRITECOMMAND(0x81);				// Contract control
	SH1107_WRITECOMMAND(0x6E);				// 128

	SH1107_WRITECOMMAND(0x20);				// Set memory addressing mode
	SH1107_WRITECOMMAND(0xA1);				// Set segment remap. Flip Horizontally with A1
	//SH1107_WRITECOMMAND(0xC0);				// Set COM scan direction. Flip Vertically with C8

	SH1107_WRITECOMMAND(0xA4);				// Normal Display
	SH1107_WRITECOMMAND(0xA6);

	SH1107_WRITECOMMAND(0xA8);				// Multiplex ratio
	SH1107_WRITECOMMAND(0x3F);				// Duty cycle =  1/64

	SH1107_WRITECOMMAND(0xD3);				// Set offset
	SH1107_WRITECOMMAND(0x60);				// Default offset

	SH1107_WRITECOMMAND(0xD5);				// Set OSC Division
	SH1107_WRITECOMMAND(0x41);

	SH1107_WRITECOMMAND(0xD9);				// Set pre-charge period
	SH1107_WRITECOMMAND(0x22);

	SH1107_WRITECOMMAND(0xDB);				// Set vcomh
	SH1107_WRITECOMMAND(0x35);

	SH1107_WRITECOMMAND(0xAD);				// Set charge pump enable
	SH1107_WRITECOMMAND(0x81);				// Set DC-DC enable (80 disables)

	SH1107_SetDisplayOn(1);					// Display on

	/* Clear screen */
	SH1107_Fill(SH1107_COLOR_BLACK);

	/* Update screen */
	SH1107_UpdateScreen();

	/* Set default values */
	SH1107.CurrentX = 0;
	SH1107.CurrentY = 0;

	/* Initialized OK */
	SH1107.Initialized = 1;

	/* Return OK */
	return 1;
}

void SH1107_UpdateScreen(void) {

	for(uint8_t i = 0; i < (SH1107HEIGHT/8); i++) {
	        SH1107_WRITECOMMAND(0xB0 + i); // Set the current RAM page address.
	        SH1107_WRITECOMMAND(0x00 + SH1107_X_OFFSET_LOWER);
	        SH1107_WRITECOMMAND(0x10 + SH1107_X_OFFSET_UPPER);
	        SH1107_WriteData(&SH1107_Buffer[SH1107WIDTH*i],SH1107WIDTH);
	    }
}

void SH1107_Fill(SH1107_COLOR_t color) {
    /* Set memory */
    uint32_t i;

    for(i = 0; i < sizeof(SH1107_Buffer); i++) {
        SH1107_Buffer[i] = (color == SH1107_COLOR_BLACK) ? 0x00 : 0xFF;
    }
}

void SH1107_DrawPixel(uint16_t x, uint16_t y, SH1107_COLOR_t color) {
	if (x >= SH1107WIDTH || y >= SH1107HEIGHT
	) {
		/* Error */
		return;
	}

	/* Check if pixels are inverted */
	if (SH1107.Inverted) {
		color = (SH1107_COLOR_t)!color;
	}

	/* Set color */
	if (color == SH1107_COLOR_WHITE) {
		SH1107_Buffer[x + (y / 8) * SH1107WIDTH] |= 1 << (y % 8);
	} else {
		SH1107_Buffer[x + (y / 8) * SH1107WIDTH] &= ~(1 << (y % 8));
	}
}

char SH1107_Putc(char ch, FontDef_t* Font, SH1107_COLOR_t color) {
	uint32_t i, b, j;

	/* Check available space in LCD */
	if (
		SH1107WIDTH <= (SH1107.CurrentX + Font->FontWidth) ||
		SH1107HEIGHT <= (SH1107.CurrentY + Font->FontHeight)
	) {
		/* Error */
		return 0;
	}

	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				SH1107_DrawPixel(SH1107.CurrentX + j, (SH1107.CurrentY + i), (SH1107_COLOR_t) color);
			} else {
				SH1107_DrawPixel(SH1107.CurrentX + j, (SH1107.CurrentY + i), (SH1107_COLOR_t)!color);
			}
		}
	}

	/* Increase pointer */
	SH1107.CurrentX += Font->FontWidth;

	/* Return character written */
	return ch;
}

char SH1107_Puts(char* str, FontDef_t* Font, SH1107_COLOR_t color) {
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (SH1107_Putc(*str, Font, color) != *str) {
			/* Return error */
			return *str;
		}

		/* Increase string pointer */
		str++;
	}

	/* Everything OK, zero should be returned */
	return *str;
}

void SH1107_Clear (void)
{
	SH1107_Fill (0);
    SH1107_UpdateScreen();
}

void SH1107_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
uint8_t dt[256];
dt[0] = reg;
uint8_t i;
for(i = 0; i < count; i++)
dt[i+1] = data[i];
HAL_I2C_Master_Transmit(&hi2c1, address, dt, count+1, 10);
}

// Write single command to I2C
void SH1107_I2C_Write(uint8_t address, uint8_t reg, uint8_t data) {
	uint8_t dt[2];
	dt[0] = reg;
	dt[1] = data;
	HAL_I2C_Master_Transmit(&hi2c1, address, dt, 2, 10);
}
