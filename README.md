# Si5351 C library
C library for the Si5351 clock generator.

The library is ready to use with the Arduino or RTOS ESP-IDF framework, but should work with others as well.
To work with this library you need an i2c bus driver, for Arduino and RTOS you can find it in the examples.
You can also use a ready-made driver and add your own section in si5351.h
Currently the library is only tested with Si5351A 10-MSOP REV-B.
