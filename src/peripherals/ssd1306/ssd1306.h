
#ifndef _SSD_1306_H_
#define _SSD_1306_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "utilities.h"
#include "i2c.h"
#include "ssd1306_types.h"
    
// Enumeration for screen colors
typedef enum {
    Black = 0x00,        // Black color, no pixel
    White = 0x01         // Pixel is set. Color depends on OLED
} SSD1306_COLOR;

typedef struct {
    uint8_t x;
    uint8_t y;
} SSD1306_VERTEX;


void SSD1306Init(void);
void SSD1306Fill(SSD1306_COLOR color);
void SSD1306UpdateScreen(void);
void SSD1306DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
char SSD1306WriteChar(char ch, FontDef Font, SSD1306_COLOR color);
char SSD1306WriteString(char *str, FontDef Font, SSD1306_COLOR color);
void SSD1306SetCursor(uint8_t x, uint8_t y);
void SSD1306Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color);
void SSD1306DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1306_COLOR color);
void SSD1306DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, SSD1306_COLOR color);
void SSD1306Polyline(const SSD1306_VERTEX *par_vertex, uint16_t par_size, SSD1306_COLOR color);
void SSD1306DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color);
void SSD1306SetContrast(const uint8_t value);
void SSD1306SetDisplayOn(const uint8_t on);
bool SSD1306_GetDisplayOn();

#ifdef __cplusplus
}
#endif

#endif // _SSD_1306_H_