double readThermocouple() {

  uint16_t temp;

  digitalWrite(CS, LOW);
  delay(1);

  // Read in 16 bits,
  //  15    = 0 always
  //  14..2 = 0.25 degree counts MSB First
  //  2     = 1 if thermocouple is open circuit
  //  1..0  = uninteresting status

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  temp = SPI.transfer(0);
  temp <<= 8;
  temp |= SPI.transfer(0);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
  
  if (temp & 0x4)
  {
    // Bit 2 indicates if the thermocouple is disconnected
    return 0;
  }

  // The lower three bits (0,1,2) are discarded status bits
  temp >>= 3;

  // The remaining bits are the number of 0.25 degree (C) counts
  return temp * 0.25;
}
