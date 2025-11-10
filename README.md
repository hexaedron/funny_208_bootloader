# funny_208_bootloader

A tiny and simple bootloader for the CH32V208 chip, capable of performing firmware upgrades. It fits within a 1KB size.

This bootloader depends on the [ch32fun](https://github.com/cnlohr/ch32fun) library.

## Usage

1. Flash the bootloader firmware to address `0x08000000`.
2. **Important**: The main firmware should be flashed to `0x08000400` and linked with `ORIGIN=0x00000400`.

### Firmware Upgrade Process

In your main firmware, implement the logic to fetch and place the new firmware at `0x08020000`. All firmware verification should be performed by the main firmware, as the bootloader does not perform any checks. Afterward, reboot the chip, and the upgrade process will start automatically.

## Configuration

- `#define DELAY_MS 3000`  
  Sets the bootloader delay duration (in milliseconds).

- `#define NOBLINK`  
  By default, the bootloader blinks the PC0 and PC1 LEDs (used for Ethernet on cheap Aliexpress boards) while flashing. Defining `NOBLINK` will disable this blinking, saving some space.

- `#define NOWAIT`  
  Omits the wait time entirely, which can help save both time and space during the boot process.
