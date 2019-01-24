
#include <Avnet_GFX.h>
#include <oledb_ssd1306.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH  96 // OLED display width, in pixels
#define SCREEN_HEIGHT 39 // OLED display height, in pixels

#define delay(x)	(usleep(x*1000))
#define max(x,y)        ((x>y)?x:y)

//GPIO_PIN_95 = reset for click module #2
//GPIO_PIN_2 = reset for click module #1

OLEDB_SSD1306 display(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT, GPIO_PIN_95, GPIO_PIN_96);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

static unsigned char logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

void testdrawline() 
{
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display(false); // Update screen with each newly-drawn line
    delay(100);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display(false);
    delay(100);
  }

  display.clearDisplay();

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display(false);
    delay(100);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display(false);
    delay(100);
  }

  display.clearDisplay();

  for(i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display(false);
    delay(100);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display(false);
    delay(100);
  }

  display.clearDisplay();

  for(i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display(false);
    delay(100);
  }
  for(i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE);
    display.display(false);
    delay(100);
  }

  sleep(2); // Pause for 2 seconds
}

void testdrawrect(void) 
{
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display(false); // Update screen with each newly-drawn rectangle
    delay(500);
  }

  sleep(2);
}

void testfillrect(void) 
{
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=1/*3*/) {
    // The INVERSE color is used so rectangles alternate white/black
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, INVERSE);
    display.display(false); // Update screen with each newly-drawn rectangle
    delay(500);
  }

  sleep(2);
}

void testdrawcircle(void) 
{
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display(false);
    delay(500);
  }

  sleep(2);
}

void testfillcircle(void) 
{
  display.clearDisplay();

  for(int16_t i=max(display.width(),display.height())/2-3; i>0; i-=2 /*3*/) {
    // The INVERSE color is used so circles alternate white/black
    display.fillCircle(display.width() / 2, display.height() / 2, i, INVERSE);
    display.display(false); // Update screen with each newly-drawn circle
    delay(500);
  }

  sleep(2);
}

void testdrawroundrect(void) 
{
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2-3; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i,
      display.height()/4, WHITE);
    display.display(false);
    delay(500);
  }

  sleep(2);
}

void testfillroundrect(void) 
{
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2-3; i+=2) {
    // The INVERSE color is used so round-rects alternate white/black
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i,
      display.height()/4, INVERSE);
    display.display(false);
    delay(500);
  }

  sleep(2);
}

void testdrawtriangle(void) 
{
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(
      display.width()/2  , display.height()/2-i,
      display.width()/2-i, display.height()/2+i,
      display.width()/2+i, display.height()/2+i, WHITE);
    display.display(false);
    delay(500);
  }

  sleep(2);
}

void testfilltriangle(void) 
{
  display.clearDisplay();

  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=2 /*5*/) {
    // The INVERSE color is used so triangles alternate white/black
    display.fillTriangle(
      display.width()/2  , display.height()/2-i,
      display.width()/2-i, display.height()/2+i,
      display.width()/2+i, display.height()/2+i, INVERSE);
    display.display(false);
    delay(500);
  }

  sleep(2);
}

void testdrawchar(void) 
{
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display(true);
  sleep(2);
}

void testdrawstyles(void) 
{
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.printText("Hello, World!\n");

  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.printText("%1.4f\n",3.141592);

  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.printText("%8X",0xDEADBEEF);

  display.display(true);
  sleep(4);
  display.clearDisplay();
}

void testscrolltext(void) 
{
  display.clearDisplay();
  display.display(false);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.printText("scroll...");
  display.display(true);      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x07);
  sleep(2);
  display.stopscroll();
  sleep(1);
  display.startscrollleft(0x00, 0x07);
  sleep(2);
  display.stopscroll();
  sleep(1);
  display.startscrolldiagright(0x00, 0x07);
  sleep(2);
  display.startscrolldiagleft(0x00, 0x07);
  sleep(2);
  display.stopscroll();
  sleep(1);
}

void testdrawbitmap(void) 
{
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display(true);
}

#define XPOS   0   // Indexes into the 'icons' array in function below
#define YPOS   1
#define DELTAY 2

void testanimate(uint8_t *bitmap, uint8_t w, uint8_t h) 
{
    int8_t f, t, icons[NUMFLAKES][3];

    // Initialize 'snowflake' positions
    for(f=0; f< NUMFLAKES; f++) {
        icons[f][XPOS]   = rand() % ((1 - LOGO_WIDTH) + display.width());
        icons[f][YPOS]   = rand() % ((1-LOGO_HEIGHT)  + display.height());
        icons[f][DELTAY] = rand() % (display.height() - LOGO_HEIGHT);
        }

      display.clearDisplay(); // Clear the display buffer
      for(int i=0; i<10; i++) {
          // Then update coordinates of each flake...
          for(f=0; f< NUMFLAKES; f++) {
              icons[f][YPOS] += icons[f][DELTAY];
              if (icons[f][YPOS] >= display.height()) {   // If snowflake is off the bottom of the screen...
                  icons[f][YPOS]   = 0;
                  }
              display.clearDisplay(); // Clear the display buffer
              display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, WHITE);
              display.display(true); // Show the display buffer on the screen
              icons[f][DELTAY] = rand() % ((LOGO_HEIGHT/2)+LOGO_HEIGHT);
              delay(150);        // Pause for 1/10 second
              }
          }
}


void do_oled_test() {

  printf("Running OLED test...\n");

  printf("clear the display buffer\n");
  display.clearDisplay();

  printf("Draw a single pixel in white\n");
  display.drawPixel(48, 18, WHITE);
  display.display(false);
  sleep(2);

  printf("Draw many lines\n");
  testdrawline();      // Draw many lines

  printf("Draw rectangles (outlines)\n");
  testdrawrect();      // Draw rectangles (outlines)

  printf("Draw rectangles (filled)\n");
  testfillrect();      // Draw rectangles (filled)

  printf("Draw circles (outlines)\n");
  testdrawcircle();    // Draw circles (outlines)

  printf("Draw circles (filled)\n");
  testfillcircle();    // Draw circles (filled)

  printf("Draw rounded rectangles (outlines)\n");
  testdrawroundrect(); // Draw rounded rectangles (outlines)

  printf("Draw rounded rectangles (filled)\n");
  testfillroundrect(); // Draw rounded rectangles (filled)

  printf("Draw triangles (outlines)\n");
  testdrawtriangle();  // Draw triangles (outlines)

  printf("Draw triangles (filled)\n");
  testfilltriangle();  // Draw triangles (filled)

  display.clearDisplay();
  display.display(false);

  printf("Draw characters of the default font\n");
  testdrawchar();      // Draw characters of the default font

  printf("Draw 'stylized' characters\n");
  testdrawstyles();    // Draw 'stylized' characters

  printf("Draw scrolling text\n");
  testscrolltext();    // Draw scrolling text

  printf("Draw a small bitmap image\n");
  testdrawbitmap();
  sleep(5);

  printf("Invert display\n");
  display.invertDisplay(true);
  sleep(2);

  printf("Restore display\n");
  display.invertDisplay(false);
  sleep(2);

  printf("Animation \n");
  testanimate(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT); // Animate bitmaps

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.printText("All Done!\n---------");
  display.display(true);
}

