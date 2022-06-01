#include "Display.h"

extern I2C_HandleTypeDef hi2c1;

#define Display_Offset const(0x60)		// Default offset of display
#define SH1107_WRITECOMMAND(command)	SH1107_I2C_Write(OLED_I2C, 0x00, (command))

static uint8_t SH1107_Buffer[SH1107WIDTH * SH1107HEIGHT / 8];

// Private struct for Sh1107 object
typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SH1107_t;

// Private variable
static SH1107_t SH1107;

// Constructor for the Adafruit FeatherWing 128x64 OLED
// Initialize Display
uint8_t DisplayInit(void) {

	//if(HAL_I2C_IsDeviceReady(&hi2c1, OLED_I2C, 1, 20000) != HAL_OK) {
	//	printf("I2C Not Ready\n");
	//	return(0);
	//}

	if(HAL_I2C_IsDeviceReady(&hi2c1, OLED_I2C, 1, 20000) != HAL_OK) {
			printf("I2C Not Ready\n");
			return(0);
		}

	// Init the OLED
	SH1107_WRITECOMMAND(0xAE00); 						// display off, sleep mode
	SH1107_WRITECOMMAND(0xDC0100); 						// set display start line 0
	SH1107_WRITECOMMAND(0x81014F); 						// contrast setting = 0x4f
	SH1107_WRITECOMMAND(0x2000); 						// vertical addressing mode (POR=0x20)
	SH1107_WRITECOMMAND(0xA000); 						// segment remap = 1 (POR = 0)
	SH1107_WRITECOMMAND(0xC000); 						// common output scan direction = 0
	SH1107_WRITECOMMAND(0xA8017F); 						// multiplex ratio = 128
	SH1107_WRITECOMMAND(0xD30160); 						// set display offset mode = 0x60
	SH1107_WRITECOMMAND(0xD50151); 						// divide ratio/oscillator: divide by 2
	SH1107_WRITECOMMAND(0xD90122); 						// pre-charge/dis-charge period mode: 2DCLKs/2DCKLs
	SH1107_WRITECOMMAND(0xDB0135); 						// VCOM deselect level = 0.770
	SH1107_WRITECOMMAND(0xB000); 						// set page address = 0
	SH1107_WRITECOMMAND(0xA400); 						// display off, retain RAM, normal status
	SH1107_WRITECOMMAND(0xA600); 						// normal display
	SH1107_WRITECOMMAND(0xAF00);						// DISPLAY ON

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
	uint8_t m;

	for (m = 0; m < 8; m++) {
		SH1107_WRITECOMMAND(0xB0 + m);
		SH1107_WRITECOMMAND(0x00);
		SH1107_WRITECOMMAND(0x10);

		/* Write multi data */
		SH1107_I2C_WriteMulti(OLED_I2C, 0x40, &SH1107_Buffer[SH1107WIDTH * m], SH1107WIDTH);
	}
}

void SH1107_Fill(SH1107_COLOR_t color) {
	/* Set memory */
	memset(SH1107_Buffer, (color == SH1107_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SH1107_Buffer));
}

void SH1107_DrawPixel(uint16_t x, uint16_t y, SH1107_COLOR_t color) {
	if (
		x >= SH1107WIDTH ||
		y >= SH1107HEIGHT
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
