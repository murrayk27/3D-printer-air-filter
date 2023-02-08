# 3D-printer-air-filter
Scripts for arduino uno for modulating up to 3 fan speeds based on temperature feedback. This is to be used for filtering the air of 3D printer enclosures.

## Hardware:

Arduion Uno rev3

Fans: 9S0912P4F011 Sanyo Denki (Have been replaced with: )

Temp Sensors: DS18B20

## Relevent Tutorials

Controlling the fan speeds: https://fdossena.com/?p=ArduinoFanControl/i.md

Reading the temp sensors: https://randomnerdtutorials.com/guide-for-ds18b20-temperature-sensor-with-arduino/

LCD screen control: https://howtomechatronics.com/tutorials/arduino/lcd-tutorial/

## Controller Script - fan_controller.ino

The script configures the arduino to output a 25kHz PWM signal on pins 3,9,10. Which are used to modulate the speeds of up to 3 connected fans.
