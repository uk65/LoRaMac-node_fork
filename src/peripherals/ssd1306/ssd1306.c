
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "delay.h"
#include "i2c.h"

#include "ssd1306_config.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

#ifdef PERIPHERAL_SSD1306

typedef enum {
    SSD1306_OK  = 0x00,
    SSD1306_ERR = 0x01   // Generic error.
} SSD1306_Error_t;

// Struct to store transformations
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
    uint8_t DisplayOn;
} SSD1306_t;



extern I2c_t  I2c;


#define SSD1306_I2C_ADDR      0x3C << 1   // unshifted address
#define SSD1306_BUFFER_SIZE   SSD1306_WIDTH * SSD1306_HEIGHT / 8

uint8_t send_buffer[256];

// Send a byte to the command register
static void ssd1306_WriteCommand(uint8_t byte)
{
    send_buffer[0] = 0x00;
    send_buffer[1] = byte;
    
    I2cWriteBuffer(&I2c, SSD1306_I2C_ADDR, send_buffer, 2);
}

// Send data
static void ssd1306_WriteData(uint8_t *buffer, size_t buff_size)
{
    send_buffer[0] = 0x40;
    memcpy(&send_buffer[1], buffer, buff_size);
    
    I2cWriteBuffer(&I2c, SSD1306_I2C_ADDR, send_buffer, (uint16_t) buff_size + 1);
}

// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

// Screen object
static SSD1306_t SSD1306;

/* Fills the Screenbuffer with values from a given buffer of a fixed length */
SSD1306_Error_t ssd1306_FillBuffer(uint8_t *buf, uint32_t len)
{
    SSD1306_Error_t ret = SSD1306_ERR;
    if (len <= SSD1306_BUFFER_SIZE) {
        memcpy(SSD1306_Buffer, buf, len);
        ret = SSD1306_OK;
    }
    return ret;
}

// Initialize the oled screen
void SSD1306Init(void)
{
    // Wait for the screen to boot
//    delay_ms(100);

    // Init OLED
    SSD1306SetDisplayOn(0); // display off

    ssd1306_WriteCommand(0x20); // Set Memory Addressing Mode
    ssd1306_WriteCommand(0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
                                // 10b,Page Addressing Mode (RESET); 11b,Invalid

    ssd1306_WriteCommand(0xB0); // Set Page Start Address for Page Addressing Mode,0-7

#ifdef SSD1306_MIRROR_VERT
    ssd1306_WriteCommand(0xC0); // Mirror vertically
#else
    ssd1306_WriteCommand(0xC8); // Set COM Output Scan Direction
#endif

    ssd1306_WriteCommand(0x00); //---set low column address
    ssd1306_WriteCommand(0x10); //---set high column address

    ssd1306_WriteCommand(0x40); //--set start line address - CHECK

    SSD1306SetContrast(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
    ssd1306_WriteCommand(0xA0); // Mirror horizontally
#else
    ssd1306_WriteCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
#endif

#ifdef SSD1306_INVERSE_COLOR
    ssd1306_WriteCommand(0xA7); //--set inverse color
#else
    ssd1306_WriteCommand(0xA6); //--set normal color
#endif

// Set multiplex ratio.
#if (SSD1306_HEIGHT == 128)
    // Found in the Luma Python lib for SH1106.
    ssd1306_WriteCommand(0xFF);
#else
    ssd1306_WriteCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK
#endif

#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x1F); //
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x3F); //
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x3F); // Seems to work for 128px high displays too.
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    ssd1306_WriteCommand(0xA4); // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content

    ssd1306_WriteCommand(0xD3); //-set display offset - CHECK
    ssd1306_WriteCommand(0x00); //-not offset

    ssd1306_WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
    ssd1306_WriteCommand(0xF0); //--set divide ratio

    ssd1306_WriteCommand(0xD9); //--set pre-charge period
    ssd1306_WriteCommand(0x22); //

    ssd1306_WriteCommand(0xDA); //--set com pins hardware configuration - CHECK
#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x02);
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x12);
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    ssd1306_WriteCommand(0xDB); //--set vcomh
    ssd1306_WriteCommand(0x20); // 0x20,0.77xVcc

    ssd1306_WriteCommand(0x8D); //--set DC-DC enable
    ssd1306_WriteCommand(0x14); //
    SSD1306SetDisplayOn(1);    //--turn on SSD1306 panel

    // Clear screen
    SSD1306Fill(Black);

    // Flush buffer to screen
    SSD1306UpdateScreen();

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;
}

// Fill the whole screen with the given color
void SSD1306Fill(SSD1306_COLOR color)
{
    /* Set memory */
    uint32_t i;

    for (i = 0; i < sizeof(SSD1306_Buffer); i++) {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

// Write the screenbuffer with changed to the screen
void SSD1306UpdateScreen(void)
{
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
    for (uint8_t i = 0; i < SSD1306_HEIGHT / 8; i++) {
        ssd1306_WriteCommand(0xB0 + i); // Set the current RAM page address.
        ssd1306_WriteCommand(0x00);
        ssd1306_WriteCommand(0x10);
        ssd1306_WriteData(&SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
    }
}

//    Draw one pixel in the screenbuffer
//    X => X Coordinate
//    Y => Y Coordinate
//    color => Pixel color
void SSD1306DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
        return;

    // Check if pixel should be inverted
    if (SSD1306.Inverted)
        color = (SSD1306_COLOR)!color;

    // Draw in the right color
    if (color == White)
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    else
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

// Draw 1 char to the screen buffer
// ch       => char om weg te schrijven
// Font     => Font waarmee we gaan schrijven
// color    => Black or White
char SSD1306WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
    uint32_t i, b, j;

    // Check if character is valid
    if (ch < 32 || ch > 126)
        return 0;

    // Check remaining space on current line
    if (SSD1306_WIDTH < (SSD1306.CurrentX + Font.FontWidth) ||
        SSD1306_HEIGHT < (SSD1306.CurrentY + Font.FontHeight)) {
        // Not enough space on current line
        return 0;
    }

    // Use the font to write
    for (i = 0; i < Font.FontHeight; i++) {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++) {
            if ((b << j) & 0x8000) {
                SSD1306DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)color);
            } else {
                SSD1306DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

// Write full string to screenbuffer
char SSD1306WriteString(char * str, FontDef Font, SSD1306_COLOR color)
{
    // Write until null-byte
    while (*str) {
        if (SSD1306WriteChar(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

// Position the cursor
void SSD1306SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

// Draw line by Bresenhem's algorithm
void SSD1306Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color)
{
    int32_t deltaX = abs(x2 - x1);
    int32_t deltaY = abs(y2 - y1);
    int32_t signX = ((x1 < x2) ? 1 : -1);
    int32_t signY = ((y1 < y2) ? 1 : -1);
    int32_t error = deltaX - deltaY;
    int32_t error2;

    SSD1306DrawPixel(x2, y2, color);
    
    while ((x1 != x2) || (y1 != y2)) {
        SSD1306DrawPixel(x1, y1, color);
        error2 = error * 2;
        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }

        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}

// Draw polyline
void SSD1306Polyline(const SSD1306_VERTEX * par_vertex, uint16_t par_size, SSD1306_COLOR color)
{
    uint16_t i;
    if (par_vertex != 0) {
        for (i = 1; i < par_size; i++) {
            SSD1306Line(par_vertex[i - 1].x, par_vertex[i - 1].y, par_vertex[i].x, par_vertex[i].y, color);
        }
    }
}

/*Convert Degrees to Radians*/
static float ssd1306_DegToRad(float par_deg)
{
    return par_deg * 3.14 / 180.0;
}

/*Normalize degree to [0;360]*/
static uint16_t ssd1306_NormalizeTo0_360(uint16_t par_deg)
{
    uint16_t loc_angle;

    if (par_deg <= 360)
        loc_angle = par_deg;
    else
    {
        loc_angle = par_deg % 360;
        loc_angle = ((par_deg != 0) ? par_deg : 360);
    }
    
    return loc_angle;
}

/*DrawArc. Draw angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle in degree
 * sweep in degree
 */
void SSD1306DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1306_COLOR color)
{
#define CIRCLE_APPROXIMATION_SEGMENTS 36
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1, xp2;
    uint8_t yp1, yp2;
    uint32_t count = 0;
    uint32_t loc_sweep = 0;
    float rad;

    loc_sweep = ssd1306_NormalizeTo0_360(sweep);

    count = (ssd1306_NormalizeTo0_360(start_angle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;
    
    while (count < approx_segments) {
        rad = ssd1306_DegToRad(count * approx_degree);
        xp1 = x + (int8_t)(sin(rad) * radius);
        yp1 = y + (int8_t)(cos(rad) * radius);
        count++;
        if (count != approx_segments)
            rad = ssd1306_DegToRad(count * approx_degree);
        else
            rad = ssd1306_DegToRad(loc_sweep);
        xp2 = x + (int8_t)(sin(rad) * radius);
        yp2 = y + (int8_t)(cos(rad) * radius);
        SSD1306Line(xp1, yp1, xp2, yp2, color);
    }
}

// Draw circle by Bresenhem's algorithm
void SSD1306DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, SSD1306_COLOR par_color)
{
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if ((par_x >= SSD1306_WIDTH) || (par_y >= SSD1306_HEIGHT))
        return;

    do {
        SSD1306DrawPixel(par_x - x, par_y + y, par_color);
        SSD1306DrawPixel(par_x + x, par_y + y, par_color);
        SSD1306DrawPixel(par_x + x, par_y - y, par_color);
        SSD1306DrawPixel(par_x - x, par_y - y, par_color);
        
        e2 = err;
        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if ((-x == y) && (e2 <= x))
                e2 = 0;
        }

        if (e2 > x) {
            x++;
            err = err + (x * 2 + 1);
        }
    } while (x <= 0);
}

// Draw rectangle
void SSD1306DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color)
{
    SSD1306Line(x1, y1, x2, y1, color);
    SSD1306Line(x2, y1, x2, y2, color);
    SSD1306Line(x2, y2, x1, y2, color);
    SSD1306Line(x1, y2, x1, y1, color);
}

void SSD1306SetContrast(const uint8_t value)
{
    const uint8_t kSetContrastControlRegister = 0x81;
    
    ssd1306_WriteCommand(kSetContrastControlRegister);
    ssd1306_WriteCommand(value);
}

void SSD1306SetDisplayOn(const uint8_t on)
{
    uint8_t value;
    if (on) {
        value = 0xAF; // Display on
        SSD1306.DisplayOn = 1;
    } else {
        value = 0xAE; // Display off
        SSD1306.DisplayOn = 0;
    }
    
    ssd1306_WriteCommand(value);
}

uint8_t SSD1306GetDisplayOn()
{
    return SSD1306.DisplayOn;
}

#endif // PERIPHERAL_SSD1306
