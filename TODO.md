## Improvements

- [ ] Create board, connect Pico Zero W (can it use OpenOCD?)
  - [ ] Physical power/reset button
  - [ ] Light indicator
  - [ ] Power controlled with transistor (or relay?)
- [ ] Connections:
  - [ ] AVR (requires 5V converter) - 5V, GND, MISO, MOSI, SCK, CS (?), RST, RX, TX
  - [ ] Pico Pi - SWCLI, GND, SWIO || 5V, GND, RX, TX
  - [ ] ESP32 (?)
  - [ ] Generic connectors - 5V, 3.3V, GND, SPI (MOSI, MISO, SCK, CS), I2C (SCL, SCA), 2 serials (FTDI emulator)
- [ ] Software
  - [ ] Compress and send - fast (protobuf?)
  - [ ] Detect errors
  - [ ] Reset, control power remotely
