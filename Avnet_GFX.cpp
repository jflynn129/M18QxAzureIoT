
#include "Avnet_GFX.h"
#include "glcdfont.c"
#include <stdio.h>
#include <stdarg.h>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)  (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) 
#endif

/*! -------------------------------------------------------------------------------- 
*  @brief    Instatiate a GFX context for graphics! Can only be done by a superclass
*  @param    w   Display width, in pixels
*  @param    h   Display height, in pixels
*/
Avnet_GFX::Avnet_GFX(int16_t w, int16_t h):
WIDTH(w), HEIGHT(h)
{
    _width    = WIDTH;
    _height   = HEIGHT;
    rotation  = 0;
    cursor_y  = cursor_x    = 0;
    textsize  = 1;
    textcolor = textbgcolor = 0xFFFF;
    wrap      = true;
    _cp437    = false;
}

/*! --------------------------------------------------------------------------------
*  @brief    Write a line.  Bresenham's algorithm - thx wikpedia
*   @param    x0  Start point x coordinate
*   @param    y0  Start point y coordinate
*   @param    x1  End point x coordinate
*   @param    y1  End point y coordinate
*   @param    color 16-bit 5-6-5 Color to draw with
*/
void Avnet_GFX::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
        }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
        }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) 
        ystep = 1;
    else
        ystep = -1;

    for (; x0<=x1; x0++) {
        if (steep) 
            writePixel(y0, x0, color);
        else 
            writePixel(x0, y0, color);
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
            }
        }
}

/*! --------------------------------------------------------------------------------
*  @brief    Write a pixel, overwrite in subclasses if startWrite is defined!
*   @param   x   x coordinate
*   @param   y   y coordinate
*  @param    color 16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::writePixel(int16_t x, int16_t y, uint16_t color) { drawPixel(x, y, color); }

/*! --------------------------------------------------------------------------------
*  @brief    Write a perfectly vertical line, overwrite in subclasses if startWrite is defined!
*   @param    x   Top-most x coordinate
*   @param    y   Top-most y coordinate
*   @param    h   Height in pixels
*  @param    color 16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
    // Overwrite in subclasses if startWrite is defined!
    // Can be just writeLine(x, y, x, y+h-1, color);
    // or writeFillRect(x, y, 1, h, color);
    drawFastVLine(x, y, h, color);
}

/*! --------------------------------------------------------------------------------
*  @brief    Write a perfectly horizontal line, overwrite in subclasses if startWrite is defined!
*   @param    x   Left-most x coordinate
*   @param    y   Left-most y coordinate
*   @param    w   Width in pixels
*  @param    color 16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) 
{
    // Overwrite in subclasses if startWrite is defined!
    // Example: writeLine(x, y, x+w-1, y, color);
    // or writeFillRect(x, y, w, 1, color);
    drawFastHLine(x, y, w, color);
}

/*! --------------------------------------------------------------------------------
*  @brief    Write a rectangle completely with one color, overwrite in subclasses if startWrite is defined!
*   @param    x   Top left corner x coordinate
*   @param    y   Top left corner y coordinate
*   @param    w   Width in pixels
*   @param    h   Height in pixels
*  @param    color 16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { fillRect(x,y,w,h,color); }

/*! --------------------------------------------------------------------------------
*  @brief    Draw a perfectly vertical line (this is often optimized in a subclass!)
*   @param    x   Top-most x coordinate
*   @param    y   Top-most y coordinate
*   @param    h   Height in pixels
*  @param    color 16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
    writeLine(x, y, x, y+h-1, color);
}

/*! --------------------------------------------------------------------------------
*  @brief    Draw a perfectly horizontal line (this is often optimized in a subclass!)
*   @param    x   Left-most x coordinate
*   @param    y   Left-most y coordinate
*   @param    w   Width in pixels
*  @param    color 16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) 
{
    writeLine(x, y, x+w-1, y, color);
}

/*! --------------------------------------------------------------------------------
*  @brief    Fill a rectangle completely with one color. Update in subclasses if desired!
*   @param    x   Top left corner x coordinate
*   @param    y   Top left corner y coordinate
*   @param    w   Width in pixels
*   @param    h   Height in pixels
*  @param    color 16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
    for (int16_t i=x; i<x+w; i++) {
        writeFastVLine(i, y, h, color);
    }
}

/*! --------------------------------------------------------------------------------
*  @brief    Fill the screen completely with one color. Update in subclasses if desired!
*   @param    color 16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::fillScreen(uint16_t color) { fillRect(0, 0, _width, _height, color); }

/*! --------------------------------------------------------------------------------
*  @brief    Draw a line
*   @param    x0  Start point x coordinate
*   @param    y0  Start point y coordinate
*   @param    x1  End point x coordinate
*   @param    y1  End point y coordinate
*   @param    color 16-bit 5-6-5 Color to draw with
*/
void Avnet_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
{
    // Update in subclasses if desired!
    if(x0 == x1){
        if(y0 > y1) _swap_int16_t(y0, y1);
        drawFastVLine(x0, y0, y1 - y0 + 1, color);
    } else if(y0 == y1){
        if(x0 > x1) _swap_int16_t(x0, x1);
        drawFastHLine(x0, y0, x1 - x0 + 1, color);
    } else {
        writeLine(x0, y0, x1, y1, color);
    }
}

/*! --------------------------------------------------------------------------------
*  @brief    Draw a circle outline
*   @param    x0   Center-point x coordinate
*   @param    y0   Center-point y coordinate
*   @param    r   Radius of circle
*   @param    color 16-bit 5-6-5 Color to draw with
*/
void Avnet_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) 
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    writePixel(x0  , y0+r, color);
    writePixel(x0  , y0-r, color);
    writePixel(x0+r, y0  , color);
    writePixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        writePixel(x0 + x, y0 + y, color);
        writePixel(x0 - x, y0 + y, color);
        writePixel(x0 + x, y0 - y, color);
        writePixel(x0 - x, y0 - y, color);
        writePixel(x0 + y, y0 + x, color);
        writePixel(x0 - y, y0 + x, color);
        writePixel(x0 + y, y0 - x, color);
        writePixel(x0 - y, y0 - x, color);
    }
}

/*! --------------------------------------------------------------------------------
*   @brief    Quarter-circle drawer, used to do circles and roundrects
*   @param    x0   Center-point x coordinate
*   @param    y0   Center-point y coordinate
*   @param    r   Radius of circle
*   @param    cornername  Mask bit #1 or bit #2 to indicate which quarters of the circle we're doing
*   @param    color 16-bit 5-6-5 Color to draw with
*/
void Avnet_GFX::drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) 
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            writePixel(x0 + x, y0 + y, color);
            writePixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            writePixel(x0 + x, y0 - y, color);
            writePixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            writePixel(x0 - y, y0 + x, color);
            writePixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            writePixel(x0 - y, y0 - x, color);
            writePixel(x0 - x, y0 - y, color);
        }
    }
}

/*! --------------------------------------------------------------------------------
*  @brief    Draw a circle with filled color
*   @param    x0   Center-point x coordinate
*   @param    y0   Center-point y coordinate
*   @param    r   Radius of circle
*   @param    color 16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) 
{
    writeFastVLine(x0, y0-r, 2*r+1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}


/*! --------------------------------------------------------------------------------
*   @brief  Quarter-circle drawer with fill, used for circles and roundrects
*   @param  x0       Center-point x coordinate
*   @param  y0       Center-point y coordinate
*   @param  r        Radius of circle
*   @param  corners  Mask bits indicating which quarters we're doing
*   @param  delta    Offset from center-point, used for round-rects
*   @param  color    16-bit 5-6-5 Color to fill with
*/
void Avnet_GFX::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) 
{

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;
    int16_t px    = x;
    int16_t py    = y;

    delta++; // Avoid some +1's in the loop

    while(x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        // These checks avoid double-drawing certain lines, important
        // for the SSD1306 library which has an INVERT drawing mode.
        if(x < (y + 1)) {
            if(corners & 1) writeFastVLine(x0+x, y0-y, 2*y+delta, color);
            if(corners & 2) writeFastVLine(x0-x, y0-y, 2*y+delta, color);
        }
        if(y != py) {
            if(corners & 1) writeFastVLine(x0+py, y0-px, 2*px+delta, color);
            if(corners & 2) writeFastVLine(x0-py, y0-px, 2*px+delta, color);
            py = y;
        }
        px = x;
    }
}

/*! --------------------------------------------------------------------------------
*  @brief   Draw a rectangle with no fill color
*   @param    x   Top left corner x coordinate
*   @param    y   Top left corner y coordinate
*   @param    w   Width in pixels
*   @param    h   Height in pixels
*   @param    color 16-bit 5-6-5 Color to draw with
*/
void Avnet_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
    writeFastHLine(x, y, w, color);
    writeFastHLine(x, y+h-1, w, color);
    writeFastVLine(x, y, h, color);
    writeFastVLine(x+w-1, y, h, color);
}

/*! --------------------------------------------------------------------------------
*  @brief   Draw a rounded rectangle with no fill color
*   @param    x   Top left corner x coordinate
*   @param    y   Top left corner y coordinate
*   @param    w   Width in pixels
*   @param    h   Height in pixels
*   @param    r   Radius of corner rounding
*   @param    color 16-bit 5-6-5 Color to draw with
*/
void Avnet_GFX::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) 
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if(r > max_radius) r = max_radius;
    // smarter version
    writeFastHLine(x+r  , y    , w-2*r, color); // Top
    writeFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
    writeFastVLine(x    , y+r  , h-2*r, color); // Left
    writeFastVLine(x+w-1, y+r  , h-2*r, color); // Right
    // draw four corners
    drawCircleHelper(x+r    , y+r    , r, 1, color);
    drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
    drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
    drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

/*! --------------------------------------------------------------------------------
*  @brief   Draw a rounded rectangle with fill color
*   @param    x   Top left corner x coordinate
*   @param    y   Top left corner y coordinate
*   @param    w   Width in pixels
*   @param    h   Height in pixels
*   @param    r   Radius of corner rounding
*   @param    color 16-bit 5-6-5 Color to draw/fill with
*/
void Avnet_GFX::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) 
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if(r > max_radius) r = max_radius;
    // smarter version
    writeFillRect(x+r, y, w-2*r, h, color);
    // draw four corners
    fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
    fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

/*! --------------------------------------------------------------------------------
*  @brief   Draw a triangle with no fill color
*   @param    x0  Vertex #0 x coordinate
*   @param    y0  Vertex #0 y coordinate
*   @param    x1  Vertex #1 x coordinate
*   @param    y1  Vertex #1 y coordinate
*   @param    x2  Vertex #2 x coordinate
*   @param    y2  Vertex #2 y coordinate
*   @param    color 16-bit 5-6-5 Color to draw with
*/
void Avnet_GFX::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) 
{
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

/*! --------------------------------------------------------------------------------
*  @brief     Draw a triangle with color-fill
*   @param    x0  Vertex #0 x coordinate
*   @param    y0  Vertex #0 y coordinate
*   @param    x1  Vertex #1 x coordinate
*   @param    y1  Vertex #1 y coordinate
*   @param    x2  Vertex #2 x coordinate
*   @param    y2  Vertex #2 y coordinate
*   @param    color 16-bit 5-6-5 Color to fill/draw with
*/
void Avnet_GFX::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) 
{

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
        _swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
        _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }

    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a)      a = x1;
        else if(x1 > b) b = x1;
        if(x2 < a)      a = x2;
        else if(x2 > b) b = x2;
        writeFastHLine(a, y0, b-a+1, color);
        return;
    }

    int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
    int32_t
    sa   = 0,
    sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2) last = y1;   // Include y1 scanline
    else         last = y1-1; // Skip it

    for(y=y0; y<=last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) _swap_int16_t(a,b);
        writeFastHLine(a, y, b-a+1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for(; y<=y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) _swap_int16_t(a,b);
        writeFastHLine(a, y, b-a+1, color);
    }
}

// BITMAP FUNCTIONS ----------------------------------------------------------------

/*! --------------------------------------------------------------------------------
   @brief      Draw a RAM-resident 1-bit image at the specified (x,y) position, using the specified foreground color (unset bits are transparent).
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with monochrome bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Hieght of bitmap in pixels
    @param    color 16-bit 5-6-5 Color to draw with
*/
void Avnet_GFX::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) 
{

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            if(i & 7) byte <<= 1;
            else      byte   = bitmap[j * byteWidth + i / 8];
            if(byte & 0x80) writePixel(x+i, y, color);
        }
    }
}

/*! --------------------------------------------------------------------------------
   @brief      Draw a RAM-resident 1-bit image at the specified (x,y) position, using the specified foreground (for set bits) and background (unset bits) colors.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with monochrome bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Hieght of bitmap in pixels
    @param    color 16-bit 5-6-5 Color to draw pixels with
    @param    bg 16-bit 5-6-5 Color to draw background with
*/
void Avnet_GFX::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) 
{

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            if(i & 7) byte <<= 1;
            else      byte   = bitmap[j * byteWidth + i / 8];
            writePixel(x+i, y, (byte & 0x80) ? color : bg);
        }
    }
}

/*! --------------------------------------------------------------------------------
   @brief      Draw PROGMEM-resident XBitMap Files (*.xbm), exported from GIMP. 
   Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
   C Array can be directly used with this function.
   There is no RAM-resident version of this function; if generating bitmaps
   in RAM, use the format defined by drawBitmap() and call that instead.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with monochrome bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Hieght of bitmap in pixels
    @param    color 16-bit 5-6-5 Color to draw pixels with
*/
void Avnet_GFX::drawXBitmap(int16_t x, int16_t y, uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) 
{

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            if(i & 7) byte >>= 1;
            else      byte   = (bitmap[j * byteWidth + i / 8]);
            // Nearly identical to drawBitmap(), only the bit order
            // is reversed here (left-to-right = LSB to MSB):
            if(byte & 0x01) writePixel(x+i, y, color);
        }
    }
}


/*! --------------------------------------------------------------------------------
   @brief   Draw a RAM-resident 8-bit image (grayscale) at the specified (x,y) pos.  
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with grayscale bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Hieght of bitmap in pixels
*/
void Avnet_GFX::drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h) 
{
    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            writePixel(x+i, y, bitmap[j * w + i]);
        }
    }
}


/*! --------------------------------------------------------------------------------
   @brief   Draw a RAM-resident 8-bit image (grayscale) with a 1-bit mask
   (set bits = opaque, unset bits = clear) at the specified (x,y) position.
   BOTH buffers (grayscale and mask) must be RAM-residentt, no mix-and-match
   Specifically for 8-bit display devices such as IS31FL3731; no color reduction/expansion is performed.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with grayscale bitmap
    @param    mask  byte array with mask bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
*/
void Avnet_GFX::drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h) 
{
    int16_t bw   = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t byte = 0;
    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            if(i & 7) byte <<= 1;
            else      byte   = mask[j * bw + i / 8];
            if(byte & 0x80) {
                writePixel(x+i, y, bitmap[j * w + i]);
            }
        }
    }
}


// TEXT- AND CHARACTER-HANDLING FUNCTIONS ----------------------------------

// Draw a character
/*! --------------------------------------------------------------------------------
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    c   The 8-bit font-indexed character (likely ascii)
    @param    color 16-bit 5-6-5 Color to draw chraracter with
    @param    bg 16-bit 5-6-5 Color to fill background with (if same as color, no background)
    @param    size  Font magnification level, 1 is 'original' size
*/
void Avnet_GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) 
{
    if((x >= _width)            || // Clip right
       (y >= _height)           || // Clip bottom
       ((x + 6 * size - 1) < 0) || // Clip left
       ((y + 8 * size - 1) < 0))   // Clip top
        return;

    if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

    for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
        uint8_t line = (font[c * 5 + i]);
        for(int8_t j=0; j<8; j++, line >>= 1) {
            if(line & 1) {
                if(size == 1)
                    writePixel(x+i, y+j, color);
                else
                    writeFillRect(x+i*size, y+j*size, size, size, color);
                } 
            else if(bg != color) {
                if(size == 1)
                    writePixel(x+i, y+j, bg);
                else
                    writeFillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
        }
        if(bg != color) { // If opaque, draw vertical line for last column
            if(size == 1) 
                writeFastVLine(x+5, y, 8, bg);
            else 
                writeFillRect(x+5*size, y, size, 8*size, bg);
        }

}

/*! --------------------------------------------------------------------------------
    @brief  Print one byte/character of data, used to support print()
    @param  c  The 8-bit ascii character to write
*/
size_t Avnet_GFX::write(uint8_t c) 
{
    if(c == '\n') {                        // Newline?
        cursor_x  = 0;                     // Reset x to zero,
        cursor_y += (textsize * 8);        // advance y one line
        } 
    else if(c != '\r') {                 // Ignore carriage returns
        if(wrap && ((cursor_x + textsize * 6) > _width)) { // Off right?
            cursor_x  = 0;                 // Reset x to zero,
            cursor_y += textsize * 8;      // advance y one line
            }
        drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
        cursor_x += textsize * 6;          // Advance x one char
        }
    return 1;
}

size_t  Avnet_GFX::printText(const char *fmt, ...)
{
    char buff[255];
    va_list ap;
    va_start(ap, fmt);
    vsprintf (buff,fmt, ap);
    for( size_t i=0; i<strlen(buff); i++ )
        write(buff[i]);
    va_end(ap);
    return strlen(buff);
}

/*! --------------------------------------------------------------------------------
    @brief  Set text cursor location
    @param  x    X coordinate in pixels
    @param  y    Y coordinate in pixels
*/
void Avnet_GFX::setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }

/*! --------------------------------------------------------------------------------
    @brief  Get text cursor X location
    @returns    X coordinate in pixels
*/
int16_t Avnet_GFX::getCursorX(void) { return cursor_x; }

/*! --------------------------------------------------------------------------------
    @brief      Get text cursor Y location
    @returns    Y coordinate in pixels
*/
int16_t Avnet_GFX::getCursorY(void) { return cursor_y; }

/*! --------------------------------------------------------------------------------
    @brief   Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
    @param  s  Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc
*/
void Avnet_GFX::setTextSize(uint8_t s) { textsize = (s > 0) ? s : 1; }

/*! --------------------------------------------------------------------------------
    @brief   Set text font color with transparant background
    @param   c   16-bit 5-6-5 Color to draw text with
*/
void Avnet_GFX::setTextColor(uint16_t c) 
{
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    textcolor = textbgcolor = c;
}

/*! --------------------------------------------------------------------------------
    @brief   Set text font color with transparant background
    @param   c   16-bit 5-6-5 Color to draw text with
    @param   b   16-bit 5-6-5 Color to draw text with
*/
void Avnet_GFX::setTextColor(uint16_t c, uint16_t b) 
{
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    textcolor  = c; 
    textbgcolor= b;
}

/*! --------------------------------------------------------------------------------
    @brief      Whether text that is too long should 'wrap' around to the next line.
    @param  w Set true for wrapping, false for clipping
*/
void Avnet_GFX::setTextWrap(bool w) { wrap = w; }

/*! --------------------------------------------------------------------------------
    @brief      Get rotation setting for display
    @returns    0 thru 3 corresponding to 4 cardinal rotations
*/
uint8_t Avnet_GFX::getRotation(void) { return rotation; }

/*! --------------------------------------------------------------------------------
    @brief      Set rotation setting for display
    @param  x   0 thru 3 corresponding to 4 cardinal rotations
*/
void Avnet_GFX::setRotation(uint8_t x) 
{
    rotation = (x & 3);
    switch(rotation) {
        case 0:
        case 2:
            _width  = WIDTH;
            _height = HEIGHT;
            break;
        case 1:
        case 3:
            _width  = HEIGHT;
            _height = WIDTH;
            break;
    }
}

/*! --------------------------------------------------------------------------------
    @brief Enable (or disable) Code Page 437-compatible charset.
    There was an error in glcdfont.c for the longest time -- one character
    (#176, the 'light shade' block) was missing -- this threw off the index
    of every character that followed it.  But a TON of code has been written
    with the erroneous character indices.  By default, the library uses the
    original 'wrong' behavior and old sketches will still work.  Pass 'true'
    to this function to use correct CP437 character values in your code.
    @param  x  Whether to enable (True) or not (False)
*/
void Avnet_GFX::cp437(bool x) { _cp437 = x; }


/*! --------------------------------------------------------------------------------
    @brief    Helper to determine size of a character with current font/size.
       Broke this out as it's used by both the PROGMEM- and RAM-resident getTextBounds() functions.
    @param    c     The ascii character in question
    @param    x     Pointer to x location of character
    @param    y     Pointer to y location of character
    @param    minx  Minimum clipping value for X
    @param    miny  Minimum clipping value for Y
    @param    maxx  Maximum clipping value for X
    @param    maxy  Maximum clipping value for Y
*/
void Avnet_GFX::charBounds(char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy) 
{
    if(c == '\n') {                     // Newline?
        *x  = 0;                        // Reset x to zero,
        *y += textsize * 8;             // advance y one line
        // min/max x/y unchaged -- that waits for next 'normal' character
        } 
    else if(c != '\r') {  // Normal char; ignore carriage returns
        if(wrap && ((*x + textsize * 6) > _width)) { // Off right?
            *x  = 0;                    // Reset x to zero,
            *y += textsize * 8;         // advance y one line
            }
        int x2 = *x + textsize * 6 - 1; // Lower-right pixel of char
        int y2 = *y + textsize * 8 - 1;
        if(x2 > *maxx) *maxx = x2;      // Track max x, y
        if(y2 > *maxy) *maxy = y2;
        if(*x < *minx) *minx = *x;      // Track min x, y
        if(*y < *miny) *miny = *y;
        *x += textsize * 6;             // Advance x one char
    }
}

/*! --------------------------------------------------------------------------------
    @brief    Helper to determine size of a string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
    @param    str     The ascii string to measure
    @param    x       The current cursor X
    @param    y       The current cursor Y
    @param    x1      The boundary X coordinate, set by function
    @param    y1      The boundary Y coordinate, set by function
    @param    w      The boundary width, set by function
    @param    h      The boundary height, set by function
*/
void Avnet_GFX::getTextBounds(char *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) 
{
    uint8_t c; // Current character

    *x1 = x;
    *y1 = y;
    *w  = *h = 0;

    int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

    while((c = *str++))
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);

    if(maxx >= minx) {
        *x1 = minx;
        *w  = maxx - minx + 1;
    }
    if(maxy >= miny) {
        *y1 = miny;
        *h  = maxy - miny + 1;
    }
}

/*! --------------------------------------------------------------------------------
    @brief      Get width of the display, accounting for the current rotation
    @returns    Width in pixels
*/
int16_t Avnet_GFX::width(void) { return _width; }

/*! --------------------------------------------------------------------------------
    @brief      Get height of the display, accounting for the current rotation
    @returns    Height in pixels
*/
int16_t Avnet_GFX::height(void) { return _height; }

/*! --------------------------------------------------------------------------------
    @brief      Invert the display (ideally using built-in hardware command)
    @param   i  True if you want to invert, false to make 'normal'
*/
void Avnet_GFX::invertDisplay(bool i) { /* Do nothing, must be subclassed if supported by hardware */ }



// -------------------------------------------------------------------------

// GFXcanvas1, 
//  provide 1-bit offscreen canvases, the address of which can be passed to 
// drawBitmap().
// This is here mostly to help with the recently-
// added proportionally-spaced fonts; adds a way to refresh a section of the
// screen without a massive flickering clear-and-redraw...but maybe you'll
// find other uses too.  VERY RAM-intensive, since the buffer is in MCU
// memory and not the display driver...GXFcanvas1 might be minimally useful
// on an Uno-class board, but this and the others are much more likely to
// require at least a Mega or various recent ARM-type boards (recommended,
// as the text+bitmap draw can be pokey).  GFXcanvas1 requires 1 bit per
// pixel (rounded up to nearest byte per scanline), GFXcanvas8 is 1 byte
// per pixel (no scanline pad), and GFXcanvas16 uses 2 bytes per pixel (no
// scanline pad).
// NOT EXTENSIVELY TESTED YET.  MAY CONTAIN WORST BUGS KNOWN TO HUMANKIND.

/*! --------------------------------------------------------------------------------
   @brief    Instatiate a GFX 1-bit canvas context for graphics
   @param    w   Display width, in pixels
   @param    h   Display height, in pixels
*/
GFXcanvas1::GFXcanvas1(uint16_t w, uint16_t h) : Avnet_GFX(w, h) 
{
    fillScreen(BLACK);
}

/*! --------------------------------------------------------------------------------
   @brief    Delete the canvas, free memory
*/
GFXcanvas1::~GFXcanvas1(void) { if(buffer) free(buffer); }

/*! --------------------------------------------------------------------------------
   @brief    Get a pointer to the internal buffer memory
   @returns  A pointer to the allocated buffer
*/
uint8_t* GFXcanvas1::getBuffer(void) { return buffer; }

/*! --------------------------------------------------------------------------------
   @brief    Draw a pixel to the canvas framebuffer
    @param   x   x coordinate
    @param   y   y coordinate
   @param    color 16-bit 5-6-5 Color to fill with
*/
void GFXcanvas1::drawPixel(int16_t x, int16_t y, uint16_t color) 
{
    if(buffer) {
        if((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) return;

        int16_t t;
        switch(rotation) {
            case 1:
                t = x;
                x = WIDTH  - 1 - y;
                y = t;
                break;
            case 2:
                x = WIDTH  - 1 - x;
                y = HEIGHT - 1 - y;
                break;
            case 3:
                t = x;
                x = y;
                y = HEIGHT - 1 - t;
                break;
        }

        uint8_t   *ptr  = &buffer[(x / 8) + y * ((WIDTH + 7) / 8)];
        if(color) *ptr |=   0x80 >> (x & 7);
        else      *ptr &= ~(0x80 >> (x & 7));
    }
}

/*! --------------------------------------------------------------------------------
   @brief    Fill the framebuffer completely with one color
   @param    color to fill with, WHITE/true=on, BLACK/false=off
*/
void GFXcanvas1::fillScreen(uint16_t color) 
{
    if(buffer) {
        uint16_t bytes = ((WIDTH + 7) / 8) * HEIGHT;
        memset(buffer, color ? 0xFF : 0x00, bytes);
    }
}

