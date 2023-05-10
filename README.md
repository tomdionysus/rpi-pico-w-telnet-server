# rpi-pico-w-telnet-server

A telnet server for the Raspberry Pi Pico W.

This project is intended as a base project for command/control of a Pico W over a telnet-like interface (TCP).

**Caveat** Telnet is unencrypted, everything you communicate with your Pico W is open to the world.

## Downloading

```bash
git clone https://github.com/tomdionysus/rpi-pico-w-telnet-server.git

```

## Building

```bash
mkdir build
cd build
PICO_BOARD=pico_w WIFI_SSID=<Your Wifi SSID> WIFI_PASSWORD=<Your Wifi Password> PICO_SDK_FETCH_FROM_GIT=true cmake ..
make
```

## Installing

The build process will produce a `server.uf2` in the build directory, which can be installed on the Pico W bu holding down the program button and connecting it via USB. The Pico W will turn up as a USB drive, and the uf2 file should be copied into the root of that drive. The Pico firmware will flash the software and reboot.

## Connecting

The server will connect to the specified Wifi and request an IP address. 

* You can use an IP scanner (or consult the DHCP on the router) to find the IP address.
* Or: use the hostname `picow.lan`

Make sure you have telnet installed, and then connect like so:

```bash
telnet picow.lan 22
```