
#define pgm_read_byte(addr) (*(const unsigned char *)(addr)) ///< PROGMEM workaround for non-AVR

#include <Avnet_GFX.h>
#include "OLEDB_SSD1306.h"
#include "splash.h"

// SOME DEFINES AND STATIC VARIABLES USED INTERNALLY -----------------------

#define ssd1306_swap(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) 

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for SPI SSD1306 display using WNC Hardware SPI.
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @param  spi
            Pointer to an existing SPI Class instance 
    @return OLEDB_SSD1306 object.
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
OLEDB_SSD1306::OLEDB_SSD1306(uint8_t w, uint8_t h, SPIC *spi, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin, uint32_t bitrate) :
  Avnet_GFX(w, h), spi(spi ? spi : &SPI), wire(NULL), buffer(NULL),
  mosiPin(-1), clkPin(-1), dcPin(dc_pin), csPin(cs_pin), rstPin(rst_pin) 
{
    gpio_init(GPIO_PIN_xxx, csPin);
    gpio_init(GPIO_PIN_xxx, rstPin);
}

/*!
    @brief  Destructor for OLEDB_SSD1306 object.
*/
OLEDB_SSD1306::~OLEDB_SSD1306(void) {
  if(buffer) {
    free(buffer);
    buffer = NULL;
  }
}

// LOW-LEVEL UTILS ---------------------------------------------------------

int OLEDB_SSD1306::spi_write_one( bool cmd, uint8_t b )
{
    uint16_t txdata = b << 7;
    if( cmd ) txdata |= 0x8000;
    gpio_write(csPin, GPIO_DATA_LOW);
    in i = spi_transfer(myspi, (uint8_t*)&txdata, sizeof(uint16_t), (uint8_t*)0, 0);
    gpio_write(csPin, GPIO_DATA_HIGH);
    return i;
}

int OLEDB_SSD1306::spi_write_seq( bool cmd, uint8_t b, int siz )
{
    int r=0;
    for( int i=0; i<siz && !r; i++ )
        r=spi_write_one(cmd,b);
    return r;
}


// ALLOCATE & INIT DISPLAY -------------------------------------------------

/*!
    @brief  Allocate RAM for image buffer, initialize peripherals and pins.
    @param  vcs
            VCC selection. Pass SSD1306_SWITCHCAPVCC to generate the display
            voltage (step up) from the 3.3V source, or SSD1306_EXTERNALVCC
            otherwise. Most situations with Adafruit SSD1306 breakouts will
            want SSD1306_SWITCHCAPVCC.
    @param  addr
            I2C address of corresponding SSD1306 display (or pass 0 to use
            default of 0x3C for 128x32 display, 0x3D for all others).
            SPI displays (hardware or software) do not use addresses, but
            this argument is still required (pass 0 or any value really,
            it will simply be ignored). Default if unspecified is 0.
    @param  reset
            If true, and if the reset pin passed to the constructor is
            valid, a hard reset will be performed before initializing the
            display. If using multiple SSD1306 displays on the same bus, and
            if they all share the same reset pin, you should only pass true
            on the first display being initialized, false on all others,
            else the already-initialized displays would be reset. Default if
            unspecified is true.
    @param  periphBegin
            If true, and if a hardware peripheral is being used (I2C or SPI,
            but not software SPI), call that peripheral's begin() function,
            else (false) it has already been done in one's sketch code.
            Cases where false might be used include multiple displays or
            other devices sharing a common bus, or situations on some
            platforms where a nonstandard begin() function is available
            (e.g. a TwoWire interface on non-default pins, as can be done
            on the ESP8266 and perhaps others).
    @return true on successful allocation/init, false otherwise.
            Well-behaved code should check the return value before
            proceeding.
    @note   MUST call this function before any drawing or updates!
*/
boolean OLEDB_SSD1306::begin(uint8_t vcs, uint8_t addr, boolean reset, boolean periphBegin) 
{
  if((!buffer) && !(buffer = (uint8_t *)malloc(WIDTH * ((HEIGHT + 7) / 8))))
    return false;

  clearDisplay();
  if(HEIGHT > 32) {
    drawBitmap((WIDTH - splash1_width) / 2, (HEIGHT - splash1_height) / 2,
      splash1_data, splash1_width, splash1_height, 1);
  } else {
    drawBitmap((WIDTH - splash2_width) / 2, (HEIGHT - splash2_height) / 2,
      splash2_data, splash2_width, splash2_height, 1);
  }

  vccstate = vcs;

  // Reset SSD1306 if requested and reset pin specified in constructor
  if(reset && (rstPin >= 0)) {
    pinMode(     rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(1);                   // VDD goes high at start, pause for 1 ms
    digitalWrite(rstPin, LOW);  // Bring reset low
    delay(10);                  // Wait 10 ms
    digitalWrite(rstPin, HIGH); // Bring out of reset
  }

  TRANSACTION_START

  // Init sequence
  static const uint8_t PROGMEM init1[] = {
    SSD1306_DISPLAYOFF,                   // 0xAE
    SSD1306_SETDISPLAYCLOCKDIV,           // 0xD5
    0x80,                                 // the suggested ratio 0x80
    SSD1306_SETMULTIPLEX };               // 0xA8
  ssd1306_commandList(init1, sizeof(init1));
  ssd1306_command1(HEIGHT - 1);

  static const uint8_t PROGMEM init2[] = {
    SSD1306_SETDISPLAYOFFSET,             // 0xD3
    0x0,                                  // no offset
    SSD1306_SETSTARTLINE | 0x0,           // line #0
    SSD1306_CHARGEPUMP };                 // 0x8D
  ssd1306_commandList(init2, sizeof(init2));

  ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

  static const uint8_t PROGMEM init3[] = {
    SSD1306_MEMORYMODE,                   // 0x20
    0x00,                                 // 0x0 act like ks0108
    SSD1306_SEGREMAP | 0x1,               // 0xA?
    SSD1306_COMSCANDEC };                 // 0xC8
  ssd1306_commandList(init3, sizeof(init3));

  if((WIDTH == 128) && (HEIGHT == 32)) {
    static const uint8_t PROGMEM init4a[] = {
      SSD1306_SETCOMPINS,                 // 0xDA
      0x02,
      SSD1306_SETCONTRAST,                // 0x81
      0x8F };
    ssd1306_commandList(init4a, sizeof(init4a));
  } else if((WIDTH == 128) && (HEIGHT == 64)) {
    static const uint8_t PROGMEM init4b[] = {
      SSD1306_SETCOMPINS,                 // 0xDA
      0x12,
      SSD1306_SETCONTRAST };              // 0x81
    ssd1306_commandList(init4b, sizeof(init4b));
    ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF);
  } else if((WIDTH == 96) && (HEIGHT == 16)) {
    static const uint8_t PROGMEM init4c[] = {
      SSD1306_SETCOMPINS,                 // 0xDA
      0x2,    // ada x12
      SSD1306_SETCONTRAST };              // 0x81
    ssd1306_commandList(init4c, sizeof(init4c));
    ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0xAF);
  } else {
    // Other screen varieties -- TBD
  }

  ssd1306_command1(SSD1306_SETPRECHARGE); // 0xd9
  ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
  static const uint8_t PROGMEM init5[] = {
    SSD1306_SETVCOMDETECT,               // 0xDB
    0x40,
    SSD1306_DISPLAYALLON_RESUME,         // 0xA4
    SSD1306_NORMALDISPLAY,               // 0xA6
    SSD1306_DEACTIVATE_SCROLL,
    SSD1306_DISPLAYON };                 // Main screen turn on
  ssd1306_commandList(init5, sizeof(init5));

  TRANSACTION_END

  return true; // Success
}

// DRAWING FUNCTIONS -------------------------------------------------------

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
              x = WIDTH - x - 1;
              break;
           case 2:
              x = WIDTH  - x - 1;
              y = HEIGHT - y - 1;
              break;
           case 3:
              ssd1306_swap(x, y);
              y = HEIGHT - y - 1;
              break;
           }
        switch(color) {
           case WHITE:   buffer[x + (y/8)*WIDTH] |=  (1 << (y&7)); break;
           case BLACK:   buffer[x + (y/8)*WIDTH] &= ~(1 << (y&7)); break;
           case INVERSE: buffer[x + (y/8)*WIDTH] ^=  (1 << (y&7)); break;
           }
        }
}

/*!
    @brief  Clear contents of display buffer (set all pixels to off).
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void OLEDB_SSD1306::clearDisplay(void) 
{
  memset(buffer, 0, WIDTH * ((HEIGHT + 7) / 8));
}

/*!
    @brief  Draw a horizontal line. This is also invoked by the Avnet_GFX
            library in generating many higher-level graphics primitives.
    @param  x
            Leftmost column -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @param  w
            Width of line, in pixels.
    @param  color
            Line color, one of: BLACK, WHITE or INVERT.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void OLEDB_SSD1306::drawFastHLine( int16_t x, int16_t y, int16_t w, uint16_t color) 
{
  boolean bSwap = false;
  switch(rotation) {
   case 1:
    // 90 degree rotation, swap x & y for rotation, then invert x
    bSwap = true;
    ssd1306_swap(x, y);
    x = WIDTH - x - 1;
    break;
   case 2:
    // 180 degree rotation, invert x and y, then shift y around for height.
    x  = WIDTH  - x - 1;
    y  = HEIGHT - y - 1;
    x -= (w-1);
    break;
   case 3:
    // 270 degree rotation, swap x & y for rotation,
    // then invert y and adjust y for w (not to become h)
    bSwap = true;
    ssd1306_swap(x, y);
    y  = HEIGHT - y - 1;
    y -= (w-1);
    break;
  }

  if(bSwap) drawFastVLineInternal(x, y, w, color);
  else      drawFastHLineInternal(x, y, w, color);
}

void OLEDB_SSD1306::drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) 
{

  if((y >= 0) && (y < HEIGHT)) { // Y coord in bounds?
    if(x < 0) { // Clip left
      w += x;
      x  = 0;
    }
    if((x + w) > WIDTH) { // Clip right
      w = (WIDTH - x);
    }
    if(w > 0) { // Proceed only if width is positive
      uint8_t *pBuf = &buffer[(y / 8) * WIDTH + x],
               mask = 1 << (y & 7);
      switch(color) {
       case WHITE:               while(w--) { *pBuf++ |= mask; }; break;
       case BLACK: mask = ~mask; while(w--) { *pBuf++ &= mask; }; break;
       case INVERSE:             while(w--) { *pBuf++ ^= mask; }; break;
      }
    }
  }
}

/*!
    @brief  Draw a vertical line. This is also invoked by the Avnet_GFX
            library in generating many higher-level graphics primitives.
    @param  x
            Column of display -- 0 at left to (screen width -1) at right.
    @param  y
            Topmost row -- 0 at top to (screen height - 1) at bottom.
    @param  h
            Height of line, in pixels.
    @param  color
            Line color, one of: BLACK, WHITE or INVERT.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void OLEDB_SSD1306::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
  boolean bSwap = false;
  switch(rotation) {
   case 1:
    // 90 degree rotation, swap x & y for rotation,
    // then invert x and adjust x for h (now to become w)
    bSwap = true;
    ssd1306_swap(x, y);
    x  = WIDTH - x - 1;
    x -= (h-1);
    break;
   case 2:
    // 180 degree rotation, invert x and y, then shift y around for height.
    x = WIDTH  - x - 1;
    y = HEIGHT - y - 1;
    y -= (h-1);
    break;
   case 3:
    // 270 degree rotation, swap x & y for rotation, then invert y
    bSwap = true;
    ssd1306_swap(x, y);
    y = HEIGHT - y - 1;
    break;
  }

  if(bSwap) drawFastHLineInternal(x, y, h, color);
  else      drawFastVLineInternal(x, y, h, color);
}

void OLEDB_SSD1306::drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) 
{

  if((x >= 0) && (x < WIDTH)) { // X coord in bounds?
    if(__y < 0) { // Clip top
      __h += __y;
      __y = 0;
    }
    if((__y + __h) > HEIGHT) { // Clip bottom
      __h = (HEIGHT - __y);
    }
    if(__h > 0) { // Proceed only if height is now positive
      // this display doesn't need ints for coordinates,
      // use local byte registers for faster juggling
      uint8_t  y = __y, h = __h;
      uint8_t *pBuf = &buffer[(y / 8) * WIDTH + x];

      // do the first partial byte, if necessary - this requires some masking
      uint8_t mod = (y & 7);
      if(mod) {
        // mask off the high n bits we want to set
        mod = 8 - mod;
        // note - lookup table results in a nearly 10% performance
        // improvement in fill* functions
        // uint8_t mask = ~(0xFF >> mod);
        static const uint8_t PROGMEM premask[8] =
          { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
        uint8_t mask = pgm_read_byte(&premask[mod]);
        // adjust the mask if we're not going to reach the end of this byte
        if(h < mod) mask &= (0XFF >> (mod - h));

        switch(color) {
         case WHITE:   *pBuf |=  mask; break;
         case BLACK:   *pBuf &= ~mask; break;
         case INVERSE: *pBuf ^=  mask; break;
        }
        pBuf += WIDTH;
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
              pBuf  += WIDTH; // Advance pointer 8 rows
              h     -= 8;     // Subtract 8 rows from height
            } while(h >= 8);
          } else {
            // store a local value to work with
            uint8_t val = (color != BLACK) ? 255 : 0;
            do {
              *pBuf = val;    // Set byte
              pBuf += WIDTH;  // Advance pointer 8 rows
              h    -= 8;      // Subtract 8 rows from height
            } while(h >= 8);
          }
        }

        if(h) { // Do the final partial byte, if necessary
          mod = h & 7;
          // this time we want to mask the low bits of the byte,
          // vs the high bits we did above
          // uint8_t mask = (1 << mod) - 1;
          // note - lookup table results in a nearly 10% performance
          // improvement in fill* functions
          static const uint8_t PROGMEM postmask[8] =
            { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
          uint8_t mask = pgm_read_byte(&postmask[mod]);
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
boolean OLEDB_SSD1306::getPixel(int16_t x, int16_t y) 
{
  if((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    // Pixel is in-bounds. Rotate coordinates if needed.
    switch(getRotation()) {
     case 1:
      ssd1306_swap(x, y);
      x = WIDTH - x - 1;
      break;
     case 2:
      x = WIDTH  - x - 1;
      y = HEIGHT - y - 1;
      break;
     case 3:
      ssd1306_swap(x, y);
      y = HEIGHT - y - 1;
      break;
    }
    return (buffer[x + (y / 8) * WIDTH] & (1 << (y & 7)));
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

// REFRESH DISPLAY ---------------------------------------------------------

/*!
    @brief  Push data currently in RAM to SSD1306 display.
    @return None (void).
    @note   Drawing operations are not visible until this function is
            called. Call after each graphics command, or after a whole set
            of graphics commands, as best needed by one's own application.
*/
void OLEDB_SSD1306::display(void) 
{
    uint8_t dlist[] = {
        SSD1306_PAGEADDR,
        0,                         // Page start address
        0xFF,                      // Page end (not really, but works here)
        SSD1306_COLUMNADDR,
        0                          // Column start address
        (WIDTH - 1)                // Column end address
        };
    spi_write_seq( SSD1306_COMMAND,  dlist, sizeof(dlist) );

    uint16_t count = WIDTH * ((HEIGHT + 7) / 8);
    uint8_t *ptr   = buffer;
    spi_write_seq( SSD1306_DATA,  ptr, count );
  }
}

// SCROLLING FUNCTIONS -----------------------------------------------------

/*!
    @brief  Activate a right-handed scroll for all or part of the display.
    @param  start
            First row.
    @param  stop
            Last row.
    @return None (void).
*/
// To scroll the whole display, run: display.startscrollright(0x00, 0x0F)
void OLEDB_SSD1306::startscrollright(uint8_t start, uint8_t stop) 
{
    static const uint8_t PROGMEM scrollList[] = {
        SSD1306_RIGHT_HORIZONTAL_SCROLL,
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
    @param  start
            First row.
    @param  stop
            Last row.
    @return None (void).
*/
// To scroll the whole display, run: display.startscrollleft(0x00, 0x0F)
void OLEDB_SSD1306::startscrollleft(uint8_t start, uint8_t stop) 
{
    uint8_t scrollList[] = {
        SSD1306_LEFT_HORIZONTAL_SCROLL,
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
    @param  start
            First row.
    @param  stop
            Last row.
    @return None (void).
*/
// display.startscrolldiagright(0x00, 0x0F)
void OLEDB_SSD1306::startscrolldiagright(uint8_t start, uint8_t stop) 
{
    uint8_t scrollList[] = {
        SSD1306_SET_VERTICAL_SCROLL_AREA,
        0X00,
        HEIGHT,
        SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL,
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
    @param  start
            First row.
    @param  stop
            Last row.
    @return None (void).
*/
// To scroll the whole display, run: display.startscrolldiagleft(0x00, 0x0F)
void OLEDB_SSD1306::startscrolldiagleft(uint8_t start, uint8_t stop) 
{
    uint8_t scrollList[] = {
        SSD1306_SET_VERTICAL_SCROLL_AREA,
        0X00,
        HEIGHT,
        SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL,
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
    @brief  Cease a previously-begun scrolling action.
    @return None (void).
*/
void OLEDB_SSD1306::stopscroll(void) 
{
  spi_write_one( SSD1306_COMMAND, SSD1306_DEACTIVATE_SCROLL);
}

// OTHER HARDWARE SETTINGS -------------------------------------------------

/*!
    @brief  Enable or disable display invert mode (white-on-black vs
            black-on-white).
    @param  i
            If true, switch to invert mode (black-on-white), else normal
            mode (white-on-black).
    @return None (void).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed, rather a
            different pixel mode of the display hardware is used. When
            enabled, drawing BLACK (value 0) pixels will actually draw white,
            WHITE (value 1) will draw black.
*/
void OLEDB_SSD1306::invertDisplay(boolean i) {
  spi_write_one( SSD1306_COMMAND, i ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

/*!
    @brief  Dim the display.
    @param  dim
            true to enable lower brightness mode, false for full brightness.
    @return None (void).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed.
*/
void OLEDB_SSD1306::dim(boolean dim) 
{
  uint8_t contrast;

  if(dim) {
    contrast = 0; // Dimmed display
  } else {
    contrast = (vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF;
  }
  // the range of contrast to too small to be really useful
  // it is useful to dim the display
  spi_write_one( SSD1306_COMMAND, uSSD1306_SECONTRAST );
  spi_write_one( SSD1306_COMMAND, contrast);
}







  // Init sequence
  static const uint8_t PROGMEM init_seq[] = {
    SSD1306_DISPLAYOFF,                   // 0xAE
    SSD1306_SETDISPLAYCLOCKDIV,           // 0xD5
    0x80,                                 // the suggested ratio 0x80
    SSD1306_SETMULTIPLEX,                 // 0xA8
    (HEIGHT - 1),
    SSD1306_SETDISPLAYOFFSET,             // 0xD3
    0x0,                                  // no offset
    (SSD1306_SETSTARTLINE | 0x0),         // line #0
    SSD1306_CHARGEPUMP,                   // 0x8D
  ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);
    SSD1306_MEMORYMODE,                   // 0x20
    0x00,                                 // 0x0 act like ks0108
    (SSD1306_SEGREMAP | 0x1),             // 0xA?
    SSD1306_COMSCANDEC,                   // 0xC8
    SSD1306_SETCOMPINS,                   // 0xDA
    0x2,                                  // ada x12
    SSD1306_SETCONTRAST,                  // 0x81
    ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0xAF);
    SSD1306_SETPRECHARGE,                 // 0xd9
  ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
    SSD1306_SETVCOMDETECT,                // 0xDB
    0x40,
    SSD1306_DISPLAYALLON_RESUME,          // 0xA4
    SSD1306_NORMALDISPLAY,                // 0xA6
    SSD1306_DEACTIVATE_SCROLL,            // 0x2E
    SSD1306_DISPLAYON                     // Main screen turn on
    };
