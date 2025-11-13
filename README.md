# funny_208_bootloader

A tiny and simple bootloader for the CH32V208 chip, capable of performing firmware upgrades. It fits within a 1KB size.

This bootloader depends on the [ch32fun](https://github.com/cnlohr/ch32fun) library.

## Usage

1. Flash the bootloader firmware to address `0x08000000`.
2. **Important**: The main firmware should be flashed to `0x08000400` and linked with `ORIGIN=0x00000400`.

### Firmware Upgrade Process

In your main firmware, implement the logic to fetch and place the new firmware at `0x08020000`. All firmware verification should be performed by the main firmware, as the bootloader does not perform any checks. Afterward, reboot the chip, and the upgrade process will start automatically.

- **Custom Firmware Write Location**:  
  You can specify a custom address to write the new firmware. Use `BKP->DATAR[2, 3]` to provide the 32-bit address. If these values are empty, the firmware will be written to the default address `MAIN_CODE_FLASH_ADDR`. Make sure to set the correct `ORIGIN` when linking the new firmware.

- **Custom Firmware Length**:  
  You can also define a custom firmware length. Use `BKP->DATAR[4, 5]` to specify the length of the new firmware. If these values are empty, the length will default to `NEW_FW_LENGTH`.


## Configuration

- `#define DELAY_MS 3000`  
  Sets the bootloader delay duration (in milliseconds).

- `#define NOBLINK`  
  By default, the bootloader blinks the PC0 and PC1 LEDs (used for Ethernet on cheap Aliexpress boards) while flashing. Defining `NOBLINK` will disable this blinking, saving some space.

- `#define NOWAIT`  
  Omits the wait time entirely, which can help save both time and space during the boot process.
