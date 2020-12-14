#define SER 2
#define SRCLK 3
#define RCLK 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

void setup() {
  pinMode(SER, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(RCLK, OUTPUT);
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(57600);

  printContents();

}

void printContents() {
    for (int base=0; base < 255; base += 16){
      byte data[16];
      for (int offset=0; offset < 16; offset++) {
        data[offset] = readEEPROM(base + offset);
      }
  
      char buf[80];
      sprintf(buf, "%03x: %02x %02x %02x %02x %02x %02x %02x %02x\t%02x %02x %02x %02x %02x %02x %02x %02x",
      base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
      data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    
      Serial.println(buf);
  }
}

byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin ++)
    pinMode(pin, INPUT);
  
  setAddress(address, true);
  
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    data = (data << 1) + digitalRead(pin);
  }

  return data;
}

void writeEEPROM(int address, byte data){
  setAddress(address, false);

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin ++)
    pinMode(pin, OUTPUT);

  byte msb = data & 0x80;

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin ++) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }

  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);

  byte pollBusy;
  do {
    pollBusy = readEEPROM(address) & 0x80;
    delay(1);
  } while (pollBusy != msb);

}

void writeEEPROMPage(int starting_address, byte* data_array, int num_vals) {
  // Up to the operator to ensure that page is the same
  // A6- A14 must be the same
  // 0xXXXXXXXXX111111
  setAddress(starting_address, false);
  byte data;
  int address;

  for(int pin = EEPROM_D0; pin <= EEPROM_D7; pin++)
    pinMode(pin, OUTPUT);

  for (int v = 0; v < num_vals; v++) {
    address = starting_address + v;
    data = data_array[v];
    
    setAddress(address, false);
    
    for(int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
      digitalWrite(pin, data & 1);
      data = data >> 1;
    }
  
    digitalWrite(WRITE_EN, LOW);
    delayMicroseconds(1);
    digitalWrite(WRITE_EN, HIGH);
  }

  byte msb = data_array[num_vals-1] & 0x80;
  byte pollBusy;
  do {
    pollBusy = readEEPROM(address) & 0x80;
    delay(1);
  } while (pollBusy != msb);

  
}

void setAddress(int address, bool outputEnable) {
  shiftOut(SER, SRCLK, MSBFIRST, (address >> 8) | (~outputEnable << 7)); // outputEnable is MSB of address (active-low)
  shiftOut(SER, SRCLK, MSBFIRST, address);
  digitalWrite(RCLK, LOW);
  digitalWrite(RCLK, HIGH);
  digitalWrite(RCLK, LOW);
}

void loop() {
  byte page[64];
  int serialBytesRead = 0;
  int pagesRead = 0;
  int pagesToRead = 10;

  // TODO make this more graceful
  while(true) {
    serialBytesRead = Serial.readBytes(page, sizeof(page));

    if (serialBytesRead != 0) {
      serialBytesRead = 0;

      writeEEPROMPage(pagesRead*sizeof(page), page, sizeof(page));
      pagesRead++;

      if (pagesRead == pagesToRead) break;
    }
  }

  while (true);
  
}
