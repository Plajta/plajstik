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

# Credits
Thanks to [tiny-json](https://github.com/rafagafe/tiny-json) for making this a lot easier.
Thanks to Jan Cumps for his [article](https://community.element14.com/products/raspberry-pi/b/blog/posts/raspberry-pico-c-sdk-reserve-a-flash-memory-block-for-persistent-storage) on element14 for explaining how to setup the linker to free up some space for persistent storage.
