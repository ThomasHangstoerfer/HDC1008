# HDC1008
Communicate with a TI HDC1008 sensor via i2c

This code was tested with a [Adafruit HDC1008 breakout board](https://learn.adafruit.com/adafruit-hdc1008-temperature-and-humidity-sensor-breakout).

[HDC1008 Datasheet](http://www.ti.com/lit/ds/symlink/hdc1008.pdf)


Usage:

```
hdc1008 - communicate with HDC1008-sensor via i2c - (c)2015, Thomas Hangstörfer
Usage: hdc1008 [param]
	-a <address>: address of the device on the i2c-bus (default: 0x40)
	-i <bus-num>: number of the i2c-bus (default: 1 -> /dev/i2c-1)
	-m <mode>   : mode = 0: Temperature or Humidity is acquired. (default)
	            : mode = 1: Temperature and Humidity are acquired in sequence, Temperature first.
	-e <heat>   : heat = 0: Heater disabled. (default)
	            : heat = 1: Heater enabled.
	-t : read and print temperature
	-h : read and print humidity
	-c : read configuration from device. Write config to device otherwise.
	-v : be verbose
```

