# **PD Stepper** - More Software Info

## Arduino Upload settings: ##
When uplooading software with the Arduino IDE ensure you have firtst installed the ESP32 Add-on in Arduino IDE there are [many tutorials](https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/) on how to do this.

The Board type should be set as "ESP32S3 Dev Module"

USB CDC on Boot should be set to "Enabled"

![upload settings](https://github.com/joshr120/PD-Stepper/assets/120012174/f002548a-ec56-4bae-93c7-10ec5e83b6d1)


## ESPHome: ##
The current ESPHome .yaml config file treats the TMC2209 as a a4988 driver as there is no TMC2209 intergration yet. The microsteps and PD voltage are set at startup by GPIO pins and the motor is driven via the STEP and DIR pins. A future intergration could allow for advanced TMC2209 features such as sensorless homing (Could also manually talk to the TMC2209 with the [UART Bus component](https://esphome.io/components/uart.html))

Other interfaces exposed to ESPHOME include the POWER GOOD signal, this is a signal indicating the PD Stepper is getting the requested voltage from the USB power supply. The 3 buttons on the side are also set up is binary sensors and the encoder position is read using the [AS5600 component](https://esphome.io/components/sensor/as5600.html).

