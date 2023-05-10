# rpi-pico-w-telnet-server

A telnet server for the Raspberry Pi Pico W.

This project is intended as a base project for command/control of a Pico W over a telnet-like interface (TCP).

**Caveat** Telnet is unencrypted, everything you communicate with your Pico W is open to the world.

## Installing

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

