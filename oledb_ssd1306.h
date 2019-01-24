/*!
 * @file oledb_ssd1306.hpp
 *
 */

#ifndef _OLEDB_SSD1306_hpp_
#define _OLEDB_SSD1306_hpp_

#include <stdint.h>
#include <stddef.h>
#include <spi.hpp>
#include <Avnet_GFX.h>

#ifndef __HWLIB__
extern "C" {
#include <hwlib/hwlib.h>
}
#endif

#define SSD1306_LCDWIDTH                               96 ///< MicroE OLED-B display width in pixels
#define SSD1306_LCDHEIGHT                              39 ///< MicroE OLED-B display height in pixels

#define BLACK                                           0 ///< Draw 'off' pixel
#define WHITE                                           1 ///< Draw 'on' pixel
#define INVERSE                                         2 ///< Invert pixel

#define SSD1306_MEMORYMODE                           0x20 ///< See datasheet
#define SSD1306_COLUMNADDR                           0x21 ///< See datasheet
#define SSD1306_PAGEADDR                             0x22 ///< See datasheet
#define SSD1306_SETCONTRAST                          0x81 ///< See datasheet
#define SSD1306_CHARGEPUMP                           0x8D ///< See datasheet
#define SSD1306_SEGREMAP                             0xA0 ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME                  0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON                         0xA5 ///< See datasheet
#define SSD1306_NORMALDISPLAY                        0xA6 ///< See datasheet
#define SSD1306_INVERTDISPLAY                        0xA7 ///< See datasheet
#define SSD1306_SETMULTIPLEX                         0xA8 ///< See datasheet
#define SSD1306_DISPLAYOFF                           0xAE ///< See datasheet
#define SSD1306_DISPLAYON                            0xAF ///< See datasheet
#define SSD1306_COMSCANINC                           0xC0 ///< See datasheet
#define SSD1306_COMSCANDEC                           0xC8 ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET                     0xD3 ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV                   0xD5 ///< See datasheet
#define SSD1306_SETPRECHARGE                         0xD9 ///< See datasheet
#define SSD1306_SETCOMPINS                           0xDA ///< See datasheet
#define SSD1306_SETVCOMDETECT                        0xDB ///< See datasheet

#define SSD1306_SETLOWCOLUMN                         0x00 ///< Not currently used
#define SSD1306_SETHIGHCOLUMN                        0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE                         0x40 ///< See datasheet

#define SSD1306_EXTERNALVCC                          0x01 ///< External display voltage source
#define SSD1306_SWITCHCAPVCC                         0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26 ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27 ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL                    0x2E ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL                      0x2F ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3 ///< Set scroll range

#define SSD1306_COMMAND                              0x01 ///<when sending commands
#define SSD1306_DATA                                 0x00 ///<when sending data

/*! 
    @brief  Class that for SSD1306 OLED display.
*/

class OLEDB_SSD1306 : public Avnet_GFX {
  public:
//    enum color_t  { BLACK=0, WHITE, INVERSE };

    OLEDB_SSD1306(uint8_t w, uint8_t h, gpio_pin_t rst_pin, gpio_pin_t dc_pin);
    ~OLEDB_SSD1306(void);

    bool           init( bool reset );
    void           display(bool reverse);
    void           clearDisplay(void);
    void           invertDisplay(bool i);
    void           dim(bool dim);
    void           drawPixel(int16_t x, int16_t y, uint16_t color);
    bool           getPixel(int16_t x, int16_t y);
    void           startscrollright(uint8_t start, uint8_t stop);
    void           startscrollleft(uint8_t start, uint8_t stop);
    void           startscrolldiagright(uint8_t start, uint8_t stop);
    void           startscrolldiagleft(uint8_t start, uint8_t stop);
    void           stopscroll(void);
    uint8_t       *getBuffer(void);

    virtual void   drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    virtual void   drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

 private:
    inline int     spi_write_one( uint16_t cmd, uint8_t b );
    int            spi_write_seq( uint16_t cmd, uint8_t *b, int siz );
    void           drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color);
    void           drawFastVLineInternal(int16_t x, int16_t y, int16_t h, uint16_t color);

    SPI            *spi;    
    uint8_t        *buffer;
    gpio_handle_t  rstPin;
    gpio_handle_t  dcPin;
    bool           reverse_linescan;
};

#endif // _OLEDB_SSD1306_hpp_

