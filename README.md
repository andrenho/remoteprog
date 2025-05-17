# pico-pi-upload-server
Run this server in a Raspberry Pi connected to a Pico Pi using SWD, and call a HTTP service to upload a new firmware.

## Requirements

This software is meant to run a Raspberry Pi 4 or greater. The following software is required to be installed:
 * Python 3
 * [openocd](https://github.com/raspberrypi/openocd) - to compile openocd, use the branch `sdk-2.0.0`, have `libgpiod-dev` installed first,
   and run the config command as `./configure --disable-werror --enable-bcm2835gpio`.

## Connection between Raspberry Pi and the Pi Pico

Connect the following pins:

| Raspberry PI | Pico Pi |
|--------------|---------|
| GPIO24 (18)  | SWDIO   |
| GND (20)     | GND     |
| GPIO25 (22)  | SWCLK   |

# Uploading using client/server architecture

This is useful when programming is being done in a PC (client), and uploading is being done using a Raspberry Pi via SWD (server).

## Running the server

Run in the Raspberry Pi as root:

`sudo ./pico-pi-upload-server`

This will start a new server in port 8376.

## Calling the client from shell

Run on the PC: `./pico-pi-upload-client REMOTE_URL PICO_VERSION my_firmware.elf`.

`PICO_VERSION` can be `rp2040` (Pico Pi 1) or `rp2350` (Pico Pi 2). Be sure to use the **elf** file and not the **uf2**.

## Calling the client with curl (TODO)

Run on the PC:

`curl -X POST -H "Content-Type: application/octet-stream" -d "{ \"cpu\": \"rp2350\", \"firmware\": \"$(base64 -i my_firmware.elf | tr -d '\n')\" }" http://REMOTE_URL:8376`

## Calling the client with CMake

Add the following to your CMakeLists.txt:

```cmake
add_custom_target(upload-remote
    COMMAND curl -X POST -H "Content-Type: application/octet-stream" -d "{ \"cpu\": \"rp2350\", \"firmware\": \"$(base64 -i ${CMAKE_PROJECT_NAME}.elf | tr -d '\n')\" }" http://$ENV{REMOTE_URL}:8376
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "Uploading remotely..."
    VERBATIM
)
```

And set the environment variable `REMOTE_URL`.

# Uploading directly on the Raspberry Pi

## Uploading via SWD

This is useful when both development and uploading is being done on a Raspberry Pi. The Raspberry Pi and the Pico Pi need to be
connected on the 3-pin interface described above (SWD).

Pico Pi 1:

`sudo openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg -c "program my_firmware.elf verify reset exit"`

Pico Pi 2:

`sudo openocd -f interface/raspberrypi-swd.cfg -f target/rp2350.cfg -c "adapter speed 5000" -c "rp2350.dap.core1 cortex_m reset_config sysresetreq" -c "program my_firmware.elf verify; reset; exit"`

## Uploading via USB

This can be used both in the PC or the Raspberry Pi. The computer and the Pico Pi need to be connected via a USB cable.

Connect the Pico Pi with the BOOTSEL button pressed, and then run:

`./upload-usb.sh my_firmware.uf2`

(notice that for this, the **uf2** file is used instead of **elf**)
