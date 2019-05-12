/////////////////////////////////////////////////////////////////////////
void command(uint8_t c) {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS, LOW);
  digitalWrite(DC, LOW);
  SPI.transfer(c);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void sterge_tot() {
  memset(vram, 0, WIDTH * ((HEIGHT + 7) / 8));
}

/////////////////////////////////////////////////////////////////////////
void sterge_part(uint8_t x0, uint8_t y, uint8_t x1, uint8_t h) {
  for (uint8_t j = y; j <= y + h; j++)
    for (uint8_t i = x0; i <= x1; i++)
      vram[i + (j / 8)*WIDTH] &= ~(1 << (j & 7));
}

















/////////////////////////////////////////////////////////////////////////
void start() {

  if (vram)
    free(vram);
  vram = NULL;

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  SPI.begin();

  if ((!vram) && !(vram = (uint8_t *)malloc(WIDTH * ((HEIGHT + 7) / 8))))
    for (;;)

      sterge_tot();

  digitalWrite(RES, HIGH);
  delay(1);                   // VDD goes high at start, pause for 1 ms
  digitalWrite(RES, LOW);  // Bring RESet low
  delay(10);                  // Wait 10 ms
  digitalWrite(RES, HIGH); // Bring out of RESet

  digitalWrite(CS, LOW);
  digitalWrite(DC, LOW);
  SPI.transfer(SSD1306_DISPLAYOFF);
  SPI.transfer(SSD1306_SETDISPLAYCLOCKDIV);
  SPI.transfer(0x80);
  SPI.transfer(SSD1306_SETMULTIPLEX);
  SPI.transfer(HEIGHT - 1);
  SPI.transfer(SSD1306_SETDISPLAYOFFSET);
  SPI.transfer(0x00);
  SPI.transfer(SSD1306_SETSTARTLINE | 0x0);
  SPI.transfer(SSD1306_CHARGEPUMP);
  SPI.transfer(0x14);
  SPI.transfer(SSD1306_MEMORYMODE);
  SPI.transfer(0x00);
  SPI.transfer(SSD1306_SEGREMAP | 0x1);
  SPI.transfer(SSD1306_COMSCANDEC);
  SPI.transfer(SSD1306_SETCOMPINS);
  SPI.transfer(0x12);
  SPI.transfer(SSD1306_SETCONTRAST);
  SPI.transfer(0xCF);
  SPI.transfer(SSD1306_SETPRECHARGE);
  SPI.transfer(0xF1);
  SPI.transfer(SSD1306_SETVCOMDETECT);
  SPI.transfer(0x40);
  SPI.transfer(SSD1306_DISPLAYALLON_RESUME);
  SPI.transfer(SSD1306_NORMALDISPLAY);
  SPI.transfer(SSD1306_DEACTIVATE_SCROLL);
  SPI.transfer(SSD1306_DISPLAYON);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void contrast(uint8_t contrast) {
  //default 0xCF
  //range from 0 to 255
  // the range of contrast to too small to be really useful
  // it is useful to dim the display
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS, LOW);
  digitalWrite(DC, LOW);
  SPI.transfer(SSD1306_SETCONTRAST);
  SPI.transfer(contrast);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void afisare(void) {

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS, LOW);
  digitalWrite(DC, LOW);

  SPI.transfer(SSD1306_PAGEADDR);
  SPI.transfer(0);                    // Page start addRESs
  SPI.transfer(0xFF);                    // Page end (not really, but works here)
  SPI.transfer(SSD1306_COLUMNADDR);
  SPI.transfer(0);
  SPI.transfer(WIDTH - 1); // Column end addRESs

  uint16_t count = WIDTH * ((HEIGHT + 7) / 8);
  uint8_t *ptr   = vram;

  digitalWrite(DC, HIGH);
  while (count--) {
    SPI.transfer(*ptr++);
  }

  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////////////////////////////////////////////
void pixel(int16_t x, int16_t y, uint16_t color) {
  switch (color) {
    case WHITE:   vram[x + (y / 8)*WIDTH] |=  (1 << (y & 7)); break;
    case BLACK:   vram[x + (y / 8)*WIDTH] &= ~(1 << (y & 7)); break;
    case INVERSE: vram[x + (y / 8)*WIDTH] ^=  (1 << (y & 7)); break;
  }
}

/////////////////////////////////////////////////////////////////////////

void caracter(int16_t x, int16_t y, const char c, uint8_t size, uint16_t color, uint16_t bg) {
  for (int8_t i = 0; i < 5; i++ ) { // Char bitmap = 5 columns
    uint8_t line = pgm_read_byte(&font[c * 5 + i]);
    for (int8_t j = 0; j < 8; j++, line >>= 1) {
      if (line & 1) {
        if (size == 1)
          pixel(x + i, y + j, color);
        else
          fillRect(x + i * size, y + j * size, size, size, color);
      }
      else if (bg != color) {
        if (size == 1)
          pixel(x + i, y + j, bg);
        else
          fillRect(x + i * size, y + j * size, size, size, bg);
      }
    }
  }
  if (bg != color) { // If opaque, draw vertical line for last column
    if (size == 1) linie(x + 5, y, x, y + 8 - 1, bg);
    else          fillRect(x + 5 * size, y, size, 8 * size, bg);
  }
}
/////////////////////////////////////////////////////////////////////////

void sir(int16_t x, int16_t y, const char c[], uint8_t size, uint16_t color, uint16_t bg) {
  for (uint8_t i = 0; i < strlen(c); i++) {
    caracter(x + i * 4 * (size + 1), y, c[i], size, color, bg);
  }
}

/////////////////////////////////////////////////////////////////////////



void linie(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
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

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      pixel(y0, x0, color);
    } else {
      pixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

/////////////////////////////////////////////////////////////////////////

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  for (int16_t i = x; i < x + w; i++) {
    linie(i, y, i, y + h - 1, color);//echivalent drawFastVLine(x, y, h, color);
  }
}
/////////////////////////////////////////////////////////////////////////

void numar(int16_t x, int16_t y, int val, uint8_t size, uint16_t color, uint16_t bg) {
  int i, a, j;
  if (val == 0)
    caracter(x + i++ * 4 * (size + 1), y, '0', size, color, bg);
  else {
    if (val < 0)
      caracter(x + i++ * 4 * (size + 1), y, '-', size, color, bg);
    while (val != 0) {
      a = a * 10 + val % 10;
      val /= 10;
    }
    while (a != 0) {
      caracter(x + i++ * 4 * (size + 1), y, '0' + a % 10, size, color, bg);
      a /= 10;
    }
  }
}
