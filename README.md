# wisblock-multicore-data-acquisition
Demonstrate reding postion and time data from the RAK1910 Wisblock GNSS Location Module using core 0.


## Hardware

 * RAK11310 WisBlock Core module (https://docs.rakwireless.com/Product-Categories/WisBlock/RAK11310/Datasheet/#overview)
 * RAK19001 WisBlock Dual IO Base Board (https://docs.rakwireless.com/Product-Categories/WisBlock/RAK19001/Datasheet/#wisblock-dual-io-base-board-overview)
 * RAK1910 WisBlock WisBlock GNSS Location Module (https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1910/Datasheet/)

### Default Pinout

| RAK11310 |
| ----------------- |
| GPIO 2 | I2C1SDA |
| GPIO 3 | I2C1SCL |
| GPIO 4 | UART1TX  |
| GPIO 5 | UART1RX  |
| GPIO 6 | GPS_1PPS  |
| GPIO 22 | GPS_RESET  |


## Cloning

```sh
git clone --recurse-submodules https://github.com/m-nahirny/wisblock-multicore-data-acquisition.git 
```

## Building

1. [Set up the Pico C/C++ SDK](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
2. Set `PICO_SDK_PATH`
```sh
export PICO_SDK_PATH=/path/to/pico-sdk
```
3. Create `build` dir, run `cmake` and `make`:
```
```
4. Copy example `.uf2` to RAK11310 when in BOOT mode.

