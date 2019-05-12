#include "ada.h"
#include "glcdfont.c"
#include <SPI.h>
#include <string.h>

#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

uint8_t *vram;

#define CS 10
#define DC 9
#define RES 8


void setup() {
  Serial.begin(9600);
  pinMode(DC, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(RES, OUTPUT);
  start();
  delay(500);
  sir(0, 0, "OK", 2, WHITE, BLACK);
  sir(35, 0, "123*C", 2, WHITE, BLACK);
  linie(0, 50, 127, 50, WHITE);
  afisare();
  delay(1000);
  sterge_part(30, 50, 50, 3);
  afisare();
  delay(1000);
}

void loop() {
  
}
