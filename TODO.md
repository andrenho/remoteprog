# Circuit

- Pi Zero W (or 2)
- Power control - 5V and 3.3V (use transistor to control it)
  - Show LED
- Pico SWD programmer
  - SWCLK,  GND, SWIO,   5V,    TX,     RX,     RESET
  - GPIO24, GND, GPIO25, GPIO1, GPIO14, GPIO15, GPIO12
- AVR programmer
  - MOSI,   MISO,   SCK,    RST,    5V,    GND, TX,     RX
  - GPIO18, GPIO23, GPIO24, GPIO12, GPIO1, GND, GPIO14, GPIO15
- General outputs
  - SPI master + slave
    - 5V,    GND, MOSI,   MISO,   SCK,    CS0,   CS1
    - GPIO1, GND, GPIO10, GPIO09, GPIO11, GPIO8, GPIO7    # confirm pigpio
  - SPI master + slave (3.3V)
  - I2C
    - 5V+3V,  SLA,   SCL,   GND
    - GPIO01, GPIO2, GPIO3, GND
  - Serial
    - 5V,    TX,     RX,     GND
    - GPIO1, GPIO14, GPIO15, GND
- LCD1602
  - 6 pins
    - RS,     E,      D4,    D5,    D6,    D7
    - GPIO17, GPIO27, GPIO0, GPIO5, GPIO6, GPIO13
  - Show IP
  - Status: Ok, Programming, Error
- Status LED
  - OR'd SWCLK, AVR SCK, SPI SCK, SCL

# Client/server functionality

## Client/server

- [x] Configuration file
- [x] Communication code, protobuf
- [x] Create client
  - [x] Auto-save latest IP
- [x] Test communication
- [x] Upload firmware
  - [x] Receive compresse file
  - [x] Send speed
- [x] Treat errors in client
- [ ] Add filename to message
- [ ] Validate file type
- [ ] Auto-detect firmware type (on server)
- [ ] SPI communication (simulation)
- [ ] I2C communication (simulation)

## PI

- [ ] LCD code + beeper
- [ ] Reset
- [ ] Test connection (AVR/Pico)
  - [ ] Configuration files
- [ ] Upload (AVR/Pico)
  - [ ] Upload
  - [ ] Check for general errors (software not installed, etc)
  - [ ] Add verbose mode
- [ ] SPI
- [ ] I2C
- [ ] Fuse programming
- [ ] Communicate via Serial (second UART?)
