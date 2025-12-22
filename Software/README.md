# **PD Stepper** - More Software Info

#### If you just received your PD Stepper see the [Setup Instructions](https://github.com/joshr120/PD-Stepper/tree/main/Getting%20Started) for how to get started

## Arduino Upload settings: ##
When uploading software with the Arduino IDE ensure you have first installed the ESP32 Add-on in Arduino IDE there are [many tutorials](https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/) on how to do this.

The Board type should be set as "ESP32S3 Dev Module"

USB CDC on Boot should be set to "Enabled"

![upload settings](https://github.com/joshr120/PD-Stepper/assets/120012174/f002548a-ec56-4bae-93c7-10ec5e83b6d1)


## ESPHome: ##
>If you have not used ESPHome before follow the offical [getting started guide](https://esphome.io/guides/getting_started_hassio.html). Select ESP32-S3 as the board type. The custom .yaml can then be copied from the repo into the configuration editor. Ensure you also add you WiFi SSID and password to your "Secrets".

The **PD-Stepper-Blinds-Advanced.yaml** config file uses the [custom TMC2209](https://github.com/slimcdk/esphome-custom-components/tree/master/esphome/components/tmc2209) component by [slimcdk](https://github.com/slimcdk). It also uses the AS5600 encoder so that if the blinds are manually moved it will not lose its end positions. Thanks to the custom componenet the example can also be modified to use stall gaurd for sensorless homing. PD voltage can be configured and it set an startup. 
> By default this will use the power on position as the OPEN position.

> If the motor spins but the cover stays in the "open" position it means either your encoder is not working or your motor is spinning in the opposite direction releative to your encoder. You can swap which lambda function is commented out to reverse the encoder direction. 

> You can enable sensorless homeing (Stallguard) by uncommenting line 129, you may need to tune the threshold and current values to get this to work well.

The **PD-Stepper-Blinds-Simple.yaml** config file simply treats the TMC2209 as a a4988 driver. The microsteps and PD voltage are set at startup by GPIO pins and the motor is driven via the STEP and DIR pins.

The **PD-Stepper-Position-Control.yaml** config also uses the [custom TMC2209](https://github.com/slimcdk/esphome-custom-components/tree/master/esphome/components/tmc2209) component by @slimcdk. It is a more stripped down version of PD-Stepper-Blinds-Advanced.yaml allowing you to set the position of the motor using a simple slider. A good base for use cases other than blinds.

These examples also present the buttons, LEDs, power good and bus voltages to the ESPHome interface so you can do what you like with them.

## Flashing a precompiled binary online: ##

There is a great online tool called [ESPConnect](https://thelastoutpostworkshop.github.io/microcontroller_devkit/espconnect/) which can be used to flash the board without downloading any software. 

The following steps will show you how to flash the default webserver example.

**1.** Plug the PD Stepper into a PC, Open [ESPConnect](https://thelastoutpostworkshop.github.io/microcontroller_devkit/espconnect/) and press Connect.
<img width="2000" height="1202" alt="image (3)" src="https://github.com/user-attachments/assets/3e089ffc-9281-483f-aab2-36b9300187c6" />

<br>
<br>

**2.** Select your board, COM number will be different but naming will be similar.
<img width="1416" height="872" alt="image (4)" src="https://github.com/user-attachments/assets/8eacb15c-e09e-4f5a-b8cb-4a20376150bb" />

<br>
<br>

**3.** Go to the "Flash Tools" page and select the `PD_Stepper_Web_Server.bin` file (Download from [here](https://github.com/joshr120/PD-Stepper/tree/main/Software/PD_Stepper_Web_Server)).
<img width="1965" height="1118" alt="image (5)" src="https://github.com/user-attachments/assets/2dfecb2d-70da-4b6d-8ea1-3ebc5cee3f08" />

<br>
<br>

**4.** Press flash and confirm
<img width="1685" height="437" alt="image (6)" src="https://github.com/user-attachments/assets/19edbe0c-8bba-4d89-b0dd-221bea48778e" />

<br>

Feel free to reach out at [info@thingsbyjosh.com](info@thingsbyjosh.com) if you have any problems.
