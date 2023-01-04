
#include <stdint.h>

#ifndef _SSD1306_FONTS_H_
#define _SSD1306_FONTS_H_

#include "ssd1306_config.h"
#include "ssd1306_types.h"

#ifdef SSD1306_INCLUDE_FONT_6x8
extern FontDef Font_6x8;
#endif
#ifdef SSD1306_INCLUDE_FONT_7x10
extern FontDef Font_7x10;
#endif
#ifdef SSD1306_INCLUDE_FONT_11x18
extern FontDef Font_11x18;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x26
extern FontDef Font_16x26;
#endif

#endif // _SSD1306_FONTS_H_