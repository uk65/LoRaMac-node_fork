
#ifndef _SSD_1306_TYPES_H_
#define _SSD_1306_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const uint8_t   FontWidth;     // Font width in pixels
    uint8_t         FontHeight;    // Font height in pixels
    const uint16_t *data;          // Pointer to data font data array
} FontDef;

#ifdef __cplusplus
}
#endif

#endif // _SSD_1306_TYPES_H_
