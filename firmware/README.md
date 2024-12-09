Stripped down HID example from pico-examples combined with the ADC example.

# Build
Like any other RP2040 C SDK project you need just a few commands.

Firstly create and prepare a build directory, you only need to do this the first time.
```bash
mkdir build
cd build
cmake ..
```

Then you can just build like this (inside the build folder).
```bash
make -j$(nproc)
```

And you'll end up with a `.uf2` file inside the build directory.

# Flashing
Hold down the `BOOT` button of your RP2040 board and then connect it to USB or just reset it.
It will now appear as a mass storage device on your computer, then just copy the `.uf2` to that device, the board will automatically restart to it.

# Configuring
The board can be configured of serial at all times, there's no need to hold any buttons. To make the connection a little easier you can use our tool
that automatically load the config that's already on the device. The configuration is saved as JSON and is described in [CONFIG.md](/firmware/CONFIG.md).
After saving a new JSON the board will restart itself to safely apply it.

If the app doesn't suit you (or it broke, hopefully it does not), you can use just any old serial console (`Putty` on Windows, `screen` or `minicom` on Linux).
You can load the current config file from the device using the 'LOAD' command, and save a new one by sending your desired JSON.
Everything is confirmed by sending an EOT (End of transmission, 0x04) character that can be sent from a keyboard using `Ctrl` + `D`.

## Examples
Keystrokes are shown inside like this: `<CTRL + D>`
### Reading
```
Connected
LOAD<CTRL + D>
{
  "version": 1,
  "buttons": {
    "select": 0,
    "start": 1,
    "b": 3,
    "a": 4,
    "l3": 5,
    "dpad_u": 6,
    "dpad_r": 7,
    "dpad_d": 8,
    "dpad_l": 9
  },
  "deadzone": 16.0,
  "axes": {
    "x": 1,
    "y": 0
  },
  "multiplier": {
    "x": -1.0,
    "y": 1.0
  }
}
```
### Writing
```
Connected
{
  "version": 1,
  "buttons": {
    "select": 0,
    "start": 1,
    "b": 3,
    "a": 4,
    "l3": 5,
    "dpad_u": 6,
    "dpad_r": 7,
    "dpad_d": 8,
    "dpad_l": 9
  },
  "deadzone": 16.0,
  "axes": {
    "x": 1,
    "y": 0
  },
  "multiplier": {
    "x": -1.0,
    "y": 1.0
  }
}<CTRL + D>
```

# Credits
Thanks to [tiny-json](https://github.com/rafagafe/tiny-json) for making this a lot easier.

Thanks to Jan Cumps for his [article](https://community.element14.com/products/raspberry-pi/b/blog/posts/raspberry-pico-c-sdk-reserve-a-flash-memory-block-for-persistent-storage)
on element14 for explaining how to setup the linker to free up some space for persistent storage.
