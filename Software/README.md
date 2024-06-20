# **PD Stepper** - More Software Info

## Arduino Upload settings: ##
When uplooading software with the Arduino IDE ensure you have firtst installed the ESP32 Add-on in Arduino IDE there are [many tutorials](https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/) on how to do this.

The Board type should be set as "ESP32S3 Dev Module"

USB CDC on Boot should be set to "Enabled"

![image](https://github.com/joshr120/USB-PD-Stepper-Driver/assets/120012174/e59f0777-ea70-48a8-9ecd-7ac681c9cb8d)

## Arduino Serial Bug: ##
There is currently a software bug being worked on which does not allow serial communication over USB to work at the same time as programming. A program may fail to upload if Serial.begin() has previously been called. 

There are a couple of work arounds for this;
- You can manaully enter bootloader mode by holding the BOOT button, pressing and releasing the RST button and then releasing the BOOT button.
- Alternatively software can be set up such Serial.begin() is only called on startup if a button is held at boot. This will allow USB serial commication on this power cycle but not programming (This is done in all example sketches). Could also be implemented the opposite way if you plan on using serial comms more often than uploading.
- This bug is not present when using ESPHome.

## ESPHome: ##
The current ESPHome .yaml config file treats the TMC2209 as a a4988 driver as there is no TMC2209 intergration yet. The microsteps and PD voltage are set at startup by GPIO pins and the motor is driven via the STEP and DIR pins. A future intergration could allow for advanced TMC2209 features such as sensorless homing (Could also manually talk to the TMC2209 with the [UART Bus component](https://esphome.io/components/uart.html))

Other interfaces exposed to ESPHOME include the POWER GOOD signal, this is a signal indicating the PD Stepper is getting the requested voltage from the USB power supply. The 3 buttons on the side are also set up is binary sensors and the encoder position is read using the [AS5600 component](https://esphome.io/components/sensor/as5600.html).
