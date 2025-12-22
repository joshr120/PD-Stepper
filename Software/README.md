# **PD Stepper** - More Software Info

#### If you just received your PD Stepper see the [Setup Instructions](https://github.com/joshr120/PD-Stepper/tree/main/Getting%20Started) for how to get started

## Arduino-ide Upload settings: ##
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

## Arduino-cli and Makefile: ##

Install [arduino-cli](https://arduino.github.io/arduino-cli/latest/installation/)
and ensure it is in your PATH as `arduino-cli`.

Clone this repo and cd to the Software directory and run `make init`
to install the required libraries and setup arduino-cli.yaml.
```
cd PD-Stepper/Software
make init
```

You can compile and upload sketches such as `Basic_Functionality_Test` with
```
make upload S=Basic_Functionality_Test
```

For debugging and development you can just compile with:
```
make compile S=Basic_Functionality_Test
```

Use help to see other options:
```
$ make help
Usage:
  make help
  make <target> {SKETCH|S}=<SketchDir>
Examples:
  make compile  SKETCH=Simple_Button_Control
  make c S=Simple_Button_Control

Notes:
  - <SketchDir> must be a subdir containing <SketchDir>/<SketchDir>.ino
  - Trailing '/' after the sketch name is accepted

Targets:
  help h H                `make h` Show this help
  init                    `make init` One-time: init config, install core + libs
  uninit                  `make unint` Remove all files/dirs created by running `make init`
  lib.help                `make lib.help` Show lib help
  lib.<cmd>               `make lib.<cmd> PKG=<PackageName>` Runs arduino-cli lib <cmd> "$(PKG)"
  list l                  `make l` List connected boards
  core.<cmd>              `make lib.<cmd> PKG=<PackageName>` Runs arduino-cli lib <cmd> "$(PKG)"
  compile c               `make c S=<SketchDir>` Compile a sketch
  compile_commands cc     `make cc S=<SketchDir>` Generate `build/compile_commands.json` using --only-compilation-database
  upload u                `make u S=<SketchDir>` Compile if needed and Upload, (requires S or SKETCH, optional PORT)
  monitor m               `make m {PORT=<port> BAUD=<baudrate>` Open serial monitor optional; PORT=auto-detected, BAUD=115200)
  clean cl                `make cl` Remove build artifacts
```

Here is a complete example compiling and uploading PD_Stepper_Web_Server:

```
$ make c S=PD_Stepper_Web_Server/
arduino-cli  --config-file "./arduino-cli.yaml" compile -b "esp32:esp32:esp32s3" --build-path "build" "./PD_Stepper_Web_Server" ; \

Sketch uses 809937 bytes (61%) of program storage space. Maximum is 1310720 bytes.
Global variables use 44324 bytes (13%) of dynamic memory, leaving 283356 bytes for local variables. Maximum is 327680 bytes.
$ make u S=PD_Stepper_Web_Server/
arduino-cli  --config-file "./arduino-cli.yaml" compile -b "esp32:esp32:esp32s3" --build-path "build" "./PD_Stepper_Web_Server" ; \

Sketch uses 809937 bytes (61%) of program storage space. Maximum is 1310720 bytes.
Global variables use 44324 bytes (13%) of dynamic memory, leaving 283356 bytes for local variables. Maximum is 327680 bytes.
esptool.py v4.5.1
Serial port /dev/ttyACM0
Connecting...
Chip is ESP32-S3 (revision v0.2)
Features: WiFi, BLE
Crystal is 40MHz
MAC: b4:3a:45:b8:9a:c8
Uploading stub...
Running stub...
Stub running...
Changing baud rate to 921600
Changed.
Configuring flash size...
Flash will be erased from 0x00000000 to 0x00003fff...
Flash will be erased from 0x00008000 to 0x00008fff...
Flash will be erased from 0x0000e000 to 0x0000ffff...
Flash will be erased from 0x00010000 to 0x000d5fff...
Compressed 15104 bytes to 10401...
Writing at 0x00000000... (100 %)
Wrote 15104 bytes (10401 compressed) at 0x00000000 in 0.2 seconds (effective 521.1 kbit/s)...
Hash of data verified.
Compressed 3072 bytes to 146...
Writing at 0x00008000... (100 %)
Wrote 3072 bytes (146 compressed) at 0x00008000 in 0.0 seconds (effective 576.4 kbit/s)...
Hash of data verified.
Compressed 8192 bytes to 47...
Writing at 0x0000e000... (100 %)
Wrote 8192 bytes (47 compressed) at 0x0000e000 in 0.1 seconds (effective 761.1 kbit/s)...
Hash of data verified.
Compressed 810304 bytes to 513921...
Writing at 0x00010000... (3 %)
Writing at 0x000176d1... (6 %)
Writing at 0x0001f002... (9 %)
Writing at 0x0002ac9d... (12 %)
Writing at 0x00039842... (15 %)
Writing at 0x00043128... (18 %)
Writing at 0x00049104... (21 %)
Writing at 0x0004edd1... (25 %)
Writing at 0x00054a1f... (28 %)
Writing at 0x00059eed... (31 %)
Writing at 0x0005f920... (34 %)
Writing at 0x000649d4... (37 %)
Writing at 0x00069960... (40 %)
Writing at 0x0006e89f... (43 %)
Writing at 0x0007375d... (46 %)
Writing at 0x00078771... (50 %)
Writing at 0x0007d9f0... (53 %)
Writing at 0x00082bb9... (56 %)
Writing at 0x000887d7... (59 %)
Writing at 0x0008d73b... (62 %)
Writing at 0x000926cf... (65 %)
Writing at 0x0009778b... (68 %)
Writing at 0x0009ca48... (71 %)
Writing at 0x000a1f25... (75 %)
Writing at 0x000a733d... (78 %)
Writing at 0x000ad0ac... (81 %)
Writing at 0x000b27be... (84 %)
Writing at 0x000bad16... (87 %)
Writing at 0x000c31c1... (90 %)
Writing at 0x000c84ae... (93 %)
Writing at 0x000cdeb6... (96 %)
Writing at 0x000d39ff... (100 %)
Wrote 810304 bytes (513921 compressed) at 0x00010000 in 5.2 seconds (effective 1234.9 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
New upload port: /dev/ttyACM0 (serial)
``
