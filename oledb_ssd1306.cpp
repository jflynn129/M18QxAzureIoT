
#include <Avnet_GFX.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "spi.hpp"
#include "oledb_ssd1306.h"
#include "avnet_splash.h"

// SOME DEFINES AND STATIC VARIABLES USED INTERNALLY -----------------------

#define ssd1306_swap(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) 

#define delay(x) (usleep(x*1000))

//#define REVERSE_LINESCAN    //define this if you want scanlines reversed

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for SPI SSD1306 display using WNC Hardware SPI.  Allocate RAM for image buffer, 
            initialize SPI interface, reset, and D/C pins.
    @param  w - Display width in pixels
    @param  h - Display height in pixels
*/
OLEDB_SSD1306::OLEDB_SSD1306(uint8_t w, uint8_t h, gpio_pin_t rst_pin, gpio_pin_t dc_pin ) :
  Avnet_GFX(w, h), buffer(NULL), rstPin(0), dcPin(0), reverse_linescan(false)
{
    gpio_init(rst_pin, &rstPin);
    gpio_write(rstPin,  GPIO_LEVEL_HIGH );                                  // RST is active low
    gpio_dir(rstPin, GPIO_DIR_OUTPUT);  
    gpio_init(dc_pin, &dcPin);
    gpio_write(dcPin,  GPIO_LEVEL_HIGH );                                   // D/C, HIGH=Data, LOW=Command
    gpio_dir(dcPin, GPIO_DIR_OUTPUT);  
    spi = new SPI(SPI_BUS_II, SPI_BPW_8, SPIMODE_CPOL_0_CPHA_0, 960000);    //CS, MOSI and MISO handled by hardware 
    buffer = (uint8_t *)malloc(SSD1306_LCDWIDTH * ((SSD1306_LCDHEIGHT + 7) / 8));
    init(true);
}

/*!
    @brief  Destructor for OLEDB_SSD1306 object by removing allocated object & buffer
*/
OLEDB_SSD1306::~OLEDB_SSD1306(void) {
    if( spi ) free(spi);
    if(buffer) free(buffer);
}

// LOW-LEVEL IO ---------------------------------------------------------

/*!
    @brief  send a single byte command/data to the SSD1306
    @param  cmd is either SSD1306_COMMAND or SSD1306_DATA 
    @param b is a single byte that is being sent
    @return true on success, false otherwise.
*/
int OLEDB_SSD1306::spi_write_one( uint16_t cmd, uint8_t b )
{
    if( cmd ) //if sending a Command
        gpio_write( dcPin,  GPIO_LEVEL_LOW );
    int i = spi->writeSPI((uint8_t*)&b, sizeof(uint8_t));
    if( cmd ) //always leave D/C pin in data mode
        gpio_write( dcPin,  GPIO_LEVEL_HIGH );
    return i;
}

/*!
    @brief Send a command to the SSD1306
    @param cmd is either SSD1306_COMMAND or SSD1306_DATA depending of if commands or data being sent
    @param  b is a pointer to a buffer of data/commands
    @param  siz is the number of bytes in the buffer
    @return true on success, false otherwise.
*/
int OLEDB_SSD1306::spi_write_seq( uint16_t cmd, uint8_t *b, int siz )
{
    if( cmd ) //if sending a Command
        gpio_write( dcPin,  GPIO_LEVEL_LOW );
    int r=spi->writeSPI(b,siz);
    if( cmd )
        gpio_write( dcPin,  GPIO_LEVEL_HIGH );
    return r;
}

/*!
    @brief  init this routine sets up the SSD1306 for operation
    @param  reset is true if a hard reset will be performed before initializing 
    @return true on successful allocation/init, false otherwise.
*/
bool OLEDB_SSD1306::init( bool reset )
{
    uint8_t init_seq[] = {                  // Init sequence
        SSD1306_DISPLAYOFF,                   // 0xAE
        SSD1306_SETDISPLAYCLOCKDIV,           // 0xD5
        0x80,                                 // the suggested ratio 0x80
        SSD1306_SETMULTIPLEX,                 // 0xA8
        (SSD1306_LCDHEIGHT - 1),
        SSD1306_SETDISPLAYOFFSET,             // 0xD3
        0x0,                                  // no offset
        (SSD1306_SETSTARTLINE | 0x0),         // 0x40 | line #0 
        SSD1306_CHARGEPUMP,                   // 0x8D
        0x14,

        SSD1306_MEMORYMODE,                   // 0x20
        0x00,                                 // use Horizontal addressing mode
        SSD1306_COMSCANDEC,                   // 0xC8
        SSD1306_SETCOMPINS,                   // 0xDA
        0x12,                                 // ada x12
        SSD1306_SETCONTRAST,                  // 0x81
        0xAF,
        SSD1306_SETPRECHARGE,                 // 0xd9
        0x25,
        SSD1306_SETVCOMDETECT,                // 0xDB
        0x20,
        SSD1306_DISPLAYALLON_RESUME,          // 0xA4
        SSD1306_NORMALDISPLAY,                // 0xA6
        SSD1306_DISPLAYON                     // Main screen turn on
        };

    if( reset ) {
        gpio_write( rstPin,  GPIO_LEVEL_LOW );
        delay(10);   //10 msec
        gpio_write( rstPin,  GPIO_LEVEL_HIGH );
        delay(10);   //10 msec
        }

    int x= spi_write_seq( SSD1306_COMMAND,  init_seq, sizeof(init_seq) );

    for( int i=0; i< (128 * ((96+7)/8)); i++) //clear all display RAM in the SSD1306   
        spi_write_one(SSD1306_DATA, 0x00);

    clearDisplay();
    display(false);

    return x;
}

// LCD COMMAND FUNCTIONS  --------------------------------------------------

/*!
    @brief  Push data currently in RAM to SSD1306 display.
    @param  if reverse_linescan is true print buffer in reverse else normal
    @return None 
    @note   Drawing operations not visible until this function is called. If 
            REVERSE_LINESCAN is defined, the scan occures in reversed.
*/
void OLEDB_SSD1306::display(bool reverse_linescan) 
{
    uint8_t dlistrev[] = {
            (SSD1306_SEGREMAP | 1),    // 0xA1 reverse line scan   
            SSD1306_PAGEADDR,          // 0x22
            0,                         // Page start address
            (0xFF),                    // Page end (not really, but this works)
            SSD1306_COLUMNADDR,        // 0x21
            (127-(SSD1306_LCDWIDTH-1)),// Column start address if reversed
            (127)                      // Column end address
            };
    uint8_t dlistnom[] = {
            (SSD1306_SEGREMAP),        // 0xA0 normal line scan   
            SSD1306_PAGEADDR,          // 0x22
            0,                         // Page start address
            (0xFF),                    // Page end (not really, but this works)
            SSD1306_COLUMNADDR,        // 0x21
            0,                         // Column start address
            (SSD1306_LCDWIDTH-1)       // Column end address
            };


    if( reverse_linescan ) 
        spi_write_seq( SSD1306_COMMAND,  dlistrev, sizeof(dlistrev) );
    else
        spi_write_seq( SSD1306_COMMAND,  dlistnom, sizeof(dlistnom) );

    uint16_t count = SSD1306_LCDWIDTH * ((SSD1306_LCDHEIGHT + 7) / 8);
    uint8_t *ptr   = buffer;
    spi_write_seq( SSD1306_DATA,  ptr, count );
}


/*!
    @brief  Activate a right-handed scroll for all or part of the display.
    @param  start - First row.
    @param  stop  - Last row.
    @return None
    @note   To scroll the whole display, run: display.startscrollright(0x00, 0x0F)
*/
void OLEDB_SSD1306::startscrollright(uint8_t start, uint8_t stop) 
{
    uint8_t scrollList[] = {
        SSD1306_RIGHT_HORIZONTAL_SCROLL,  //0x26
        0X00,
        start,
        0X00,
        stop,
        0X00,
        0XFF,
        SSD1306_ACTIVATE_SCROLL 
        };
    spi_write_seq( SSD1306_COMMAND,  scrollList, sizeof(scrollList) );
}

/*!
    @brief  Activate a left-handed scroll for all or part of the display.
    @param  start - First row.
    @param  stop  - Last row.
    @return None
    @note   To scroll the whole display, run: display.startscrollleft(0x00, 0x0F)
*/
void OLEDB_SSD1306::startscrollleft(uint8_t start, uint8_t stop) 
{
    uint8_t scrollList[] = {
        SSD1306_LEFT_HORIZONTAL_SCROLL, //0x27
        0X00,
        start,
        0X00,
        stop,
        0X00,
        0XFF,
        SSD1306_ACTIVATE_SCROLL 
        };
    spi_write_seq( SSD1306_COMMAND,  scrollList, sizeof(scrollList) );
}

/*!
    @brief  Activate a diagonal scroll for all or part of the display.
    @param  start - First row.
    @param  stop  - Last row.
    @return None 
*/
void OLEDB_SSD1306::startscrolldiagright(uint8_t start, uint8_t stop) 
{
    uint8_t scrollList[] = {
        SSD1306_SET_VERTICAL_SCROLL_AREA,              //0xA3
        0X00,
        SSD1306_LCDHEIGHT,
        SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL,  //0x29
        0X00,
        start,
        0X00,
        stop,
        0X01,
        SSD1306_ACTIVATE_SCROLL 
        };
    spi_write_seq( SSD1306_COMMAND,  scrollList, sizeof(scrollList) );
}

/*!
    @brief  Activate alternate diagonal scroll for all or part of the display.
    @param  start - First row.
    @param  stop  - Last row.
    @return None
    @note   To scroll the whole display, run: display.startscrolldiagleft(0x00, 0x0F)
*/
void OLEDB_SSD1306::startscrolldiagleft(uint8_t start, uint8_t stop) 
{
    uint8_t scrollList[] = {
        SSD1306_SET_VERTICAL_SCROLL_AREA,             //0xA3
        0X00,
        SSD1306_LCDHEIGHT,
        SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL,  //0x2A
        0X00,
        start,
        0X00,
        stop,
        0X01,
        SSD1306_ACTIVATE_SCROLL 
        };

    spi_write_seq( SSD1306_COMMAND,  scrollList, sizeof(scrollList) );
}

/*!
    @brief  stop previously-begun scrolling action.
    @return None (void).
*/
void OLEDB_SSD1306::stopscroll(void) 
{
    spi_write_one( SSD1306_COMMAND, SSD1306_DEACTIVATE_SCROLL);
}

/*!
    @brief  Enable/disable display invert mode (white-on-black vs black-on-white).
    @param  i - If true, switch to invert (black-on-white), else normal (white-on-black).
    @return None 
    @note   buffer contents are not changed, display hardware is used.
*/
void OLEDB_SSD1306::invertDisplay(bool i) {
    spi_write_one( SSD1306_COMMAND, i ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

/*!
    @brief  Dim the display.
    @param  dim - true to enable lower brightness mode, false for full brightness.
    @return None 
    @note   buffer contents are not changed.
*/
void OLEDB_SSD1306::dim(bool dim) 
{
    spi_write_one( SSD1306_COMMAND, SSD1306_SETCONTRAST );
    spi_write_one( SSD1306_COMMAND, (dim)? 0:0xCF );
}


// DRAWING FUNCTIONS -------------------------------------------------------

/*!
    @brief  Clear contents of display buffer (set all pixels to off).
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
*/
void OLEDB_SSD1306::clearDisplay(void) 
{
  memset(buffer, 0, SSD1306_LCDWIDTH * ((SSD1306_LCDHEIGHT + 7) / 8));
}

/*!
    @brief  Set/clear/invert a single pixel. This is also invoked by the
            Avnet_GFX library in generating many higher-level graphics
            primitives.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @param  color
            Pixel color, one of: BLACK, WHITE or INVERT.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void OLEDB_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) 
{
    if((x >= 0) && (x < width()) && (y >= 0) && (y < height())) { // Pixel is in-bounds. Rotate coordinates if needed.
        switch(getRotation()) {
           case 1:
              ssd1306_swap(x, y);
              x = SSD1306_LCDWIDTH - x - 1;
              break;
           case 2:
              x = SSD1306_LCDWIDTH  - x - 1;
              y = SSD1306_LCDHEIGHT - y - 1;
              break;
           case 3:
              ssd1306_swap(x, y);
              y = SSD1306_LCDHEIGHT - y - 1;
              break;
           }
        uint8_t *byte = &buffer[x + (y/8)*SSD1306_LCDWIDTH]; // get the buffer byte needed
        switch(color) {
           case WHITE:   *byte  |=  (1 << (y&7)); break;     //set the bit within the byte
           case BLACK:   *byte  &= ~(1 << (y&7)); break;     //clr the bit within the byte
           case INVERSE: *byte  ^=  (1 << (y&7)); break;     //invert the bit within the byte
           }
        }
}

/*!
    @brief  Draw a horizontal line. This is also invoked by the Avnet_GFX
            library in generating many higher-level graphics primitives.
    @param  x - Leftmost column -- 0 at left to (screen width - 1) at right.
    @param  y - Row of display -- 0 at top to (screen height -1) at bottom.
    @param  w - Width of line, in pixels.
    @param  color - Line color, one of: BLACK, WHITE or INVERT.
    @return None 
    @note   Changes buffer contents only, no immediate effect on display.
*/
void OLEDB_SSD1306::drawFastHLine( int16_t x, int16_t y, int16_t w, uint16_t color) 
{
    bool bSwap = false;
    switch(rotation) {
        case 1: // 90 degree rotation, swap x & y for rotation, then invert x
            bSwap = true;
            ssd1306_swap(x, y);
            x = SSD1306_LCDWIDTH - x - 1;
            break;
        case 2: // 180 degree rotation, invert x and y, then shift y around for height.
            x  = SSD1306_LCDWIDTH  - x - 1;
            y  = SSD1306_LCDHEIGHT - y - 1;
            x -= (w-1);
            break;
        case 3: // 270 degree rotation, swap x & y for rotation, then invert y and adjust for w (not to become h)
            bSwap = true;
            ssd1306_swap(x, y);
            y  = SSD1306_LCDHEIGHT - y - 1;
            y -= (w-1);
            break;
    }

    if(bSwap) 
        drawFastVLineInternal(x, y, w, color);
    else      
        drawFastHLineInternal(x, y, w, color);
}

void OLEDB_SSD1306::drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) 
{
    if((y >= 0) && (y < SSD1306_LCDHEIGHT)) { // Y coord in bounds?
        if(x < 0) { // Clip left
            w += x;
            x  = 0;
            }

        if((x + w) > SSD1306_LCDWIDTH) { // Clip right
            w = (SSD1306_LCDWIDTH - x);
            }

        if(w > 0) { // Proceed only if width is positive
            uint8_t *pBuf = &buffer[(y / 8) * SSD1306_LCDWIDTH + x];
            uint8_t mask = 1 << (y & 7);
            switch(color) {
                case INVERSE:  while(w--) { *pBuf++ ^= mask; }; break;
                case WHITE:    while(w--) { *pBuf++ |= mask; }; break;
                case BLACK:    mask = ~mask; while(w--) { *pBuf++ &= mask; }; break;
                }
            }
        }
}

/*!
    @brief  Draw a vertical line. 
    @param  x - Column of display -- 0 at left to (screen width -1) at right.
    @param  y - Topmost row -- 0 at top to (screen height - 1) at bottom.
    @param  h - Height of line, in pixels.
    @param  color - Line color, one of: BLACK, WHITE or INVERT.
    @return None 
    @note   Changes buffer contents only.
*/
void OLEDB_SSD1306::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
    bool bSwap = false;
    switch(rotation) {
        case 1: // for 90 degree rotation, swap x & y, then invert x and adjust for h (to become w)
            bSwap = true;
            ssd1306_swap(x, y);
            x  = SSD1306_LCDWIDTH - x - 1;
            x -= (h-1);
            break;
        case 2: // for 180 degree rotation, invert x and y, then shift y around for height.
            x = SSD1306_LCDWIDTH  - x - 1;
            y = SSD1306_LCDHEIGHT - y - 1;
            y -= (h-1);
            break;
        case 3: // for 270 degree rotation, swap x & y for rotation, then invert y
            bSwap = true;
            ssd1306_swap(x, y);
            y = SSD1306_LCDHEIGHT - y - 1;
            break;
        }

    if(bSwap) 
        drawFastHLineInternal(x, y, h, color);
    else      
        drawFastVLineInternal(x, y, h, color);
}

void OLEDB_SSD1306::drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) 
{
    if((x >= 0) && (x < SSD1306_LCDWIDTH)) { // X coord in bounds?
        if(__y < 0) { // Clip top
            __h += __y;
            __y = 0;
            }
        if((__y + __h) > SSD1306_LCDHEIGHT) { // Clip bottom
            __h = (SSD1306_LCDHEIGHT - __y);
            }
        if(__h > 0) { // Proceed only if height is now positive
            uint8_t  y = __y, h = __h;
            uint8_t *pBuf = &buffer[(y / 8) * SSD1306_LCDWIDTH + x];

            // do the first partial byte, if necessary - this requires some masking
            uint8_t mod = (y & 7);
            if(mod) { // mask off the high n bits we want to set
                mod = 8 - mod;
                uint8_t mask = ~(0xFF >> mod);

                switch(color) {
                    case WHITE:   *pBuf |=  mask; break;
                    case BLACK:   *pBuf &= ~mask; break;
                    case INVERSE: *pBuf ^=  mask; break;
                    }
                pBuf += SSD1306_LCDWIDTH;
                }

            if(h >= mod) { // More to go?
                h -= mod;
                // Write solid bytes while we can - effectively 8 rows at a time
                if(h >= 8) {
                    if(color == INVERSE) {
                        // separate copy of the code so we don't impact performance of
                        // black/white write version with an extra comparison per loop
                        do {
                            *pBuf ^= 0xFF;  // Invert byte
                            pBuf  += SSD1306_LCDWIDTH; // Advance pointer 8 rows
                            h     -= 8;     // Subtract 8 rows from height
                            } 
                        while(h >= 8);
                        } 
                    else{
                        // store a local value to work with
                        uint8_t val = (color != BLACK) ? 255 : 0;
                        do {
                            *pBuf = val;    // Set byte
                            pBuf += SSD1306_LCDWIDTH;  // Advance pointer 8 rows
                            h    -= 8;      // Subtract 8 rows from height
                            } 
                        while(h >= 8);
                        }
                    }

                if(h){ // Do the final partial byte, if necessary
                    mod = h & 7;
                    uint8_t mask = (1 << mod) - 1;
                    switch(color) {
                        case WHITE:   *pBuf |=  mask; break;
                        case BLACK:   *pBuf &= ~mask; break;
                        case INVERSE: *pBuf ^=  mask; break;
                        }
                    }
                }
            } // endif positive height
        } // endif x in bounds
}

/*!
    @brief  Return color of a single pixel in display buffer.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @return true if pixel is set (usually WHITE, unless display invert mode
            is enabled), false if clear (BLACK).
    @note   Reads from buffer contents; may not reflect current contents of
            screen if display() has not been called.
*/
bool OLEDB_SSD1306::getPixel(int16_t x, int16_t y) 
{
    if((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
        // Pixel is in-bounds. Rotate coordinates if needed.
        switch(getRotation()) {
         case 1:
             ssd1306_swap(x, y);
             x = SSD1306_LCDWIDTH - x - 1;
             break;
         case 2:
             x = SSD1306_LCDWIDTH  - x - 1;
             y = SSD1306_LCDHEIGHT - y - 1;
             break;
         case 3:
             ssd1306_swap(x, y);
             y = SSD1306_LCDHEIGHT - y - 1;
             break;
             }
        return (buffer[x + (y / 8) * SSD1306_LCDWIDTH] & (1 << (y & 7)));
        }
    return false; // Pixel out of bounds
}

/*!
    @brief  Get base address of display buffer for direct reading or writing.
    @return Pointer to an unsigned 8-bit array, column-major, columns padded
            to full byte boundary if needed.
*/
uint8_t *OLEDB_SSD1306::getBuffer(void) 
{
    return buffer;
}

