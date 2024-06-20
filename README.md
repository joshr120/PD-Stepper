# **PD Stepper** - USB PD Closed Loop Stepper Driver & Controller
PD Stepper is a USB PD Powered Nema 17 stepper driver using the Silent Trinamic Driver TMC2209 controlled with a ESP32-S3 for IoT and other applications.

<p float="left">
  <img src="https://github.com/joshr120/USB-PD-Stepper-Driver/assets/120012174/ba7bc5b1-e88a-49f0-8844-0d211900b3ac" width="48%" />
  <img src="https://github.com/joshr120/USB-PD-Stepper-Driver/assets/120012174/39841e2b-00d1-4257-9e5e-37f5bc7c2a4f" width="48%" /> 
</p>

<p float="left">
  <img src="https://github.com/joshr120/USB-PD-Stepper-Driver/assets/120012174/d076abd4-243e-4dc6-b90f-01bfe40eda26" width="51%" />
  <img src="https://github.com/joshr120/USB-PD-Stepper-Driver/assets/120012174/720177eb-e481-40ec-9c04-c6366b8985e7" width="37%" /> 
</p>


Stepper motors are fantastic for projects, but assembling the necessary components—like a microcontroller, stepper driver, power supplies for both motor and microcontroller, and optionally an encoder can lead to bulkiness, high costs, complexity, and challenges in integrating them into compact or space-constrained designs. This project addresses these issues by consolidating everything onto a single board, eliminating the drawbacks with little to no compromises.


The original inspiration for this project was to make a compact USB PD powered stepper driver for home automation of blinds. A whole lot of features were then added and this project now aims to create a versatile stepper motor driver and controller powered by USB Power Delivery (USB PD). By leveraging the capabilities of the ESP32-S3 microcontroller, along with the precision of the TMC2209 stepper motor driver and the sensing capabilities of the AS5600 magnetic rotary position sensor, this system provides a compact and flexible solution for a wide range of stepper motor applications.

## Main Features: ##
- **TMC2209 Stepper Motor Driver:** Incorporates the TMC2209 stepper motor driver, renowned for its silent operation, high precision, and advanced features such as stealthChop™, spreadCycle™ and sensorless homing, ensuring smooth silent and efficient motor control.
- **USB PD Power:** Utilizes the USB PD standard for power delivery, giving high power, a USB-C connector and no bulky 12 and 24V power bricks needed. It ensures compatibility with a variety of power sources and enabling seamless integration into existing setups. USB PD along with the TMC2209 can drive a stepper motor with upto ~50W of power.
- **Form Factor:** Super compact forma factor, designed to fit on the back of a common Nema 17 stepper motor
- **Onboard Rotary Position Sensor:** Integrates the AS5600 magnetic rotary position sensor for accurate and reliable absolute angle measurement, enabling precise positioning and closed loop control of the stepper motor.
- **ESP32-S3 Control:** The ESP32-S3 microcontroller serves as the brains of the system, providing ample processing power, built-in Wi-Fi and Bluetooth connectivity, and a rich ecosystem of development tools and libraries for easy customization and expansion such as **ESPHome** and **ESPNow**.
- Software or hardware selectable voltage: 5V, 9V, 12V, 15V or 20V all via USB PD
- Auxiliary connectors and buttons for flexablity and expansion
- Onboard 3.3V buck converter
- Motor temperature measurment

## Current use cases & examples in the works: ##
- Home automation with ESPHome and home assistant
- Take in standard Step and Direction inputs and incorperate in closed loop control
- Wireless position copying from one encoder to another motor
- Camera slider with web interface for control and sensorlesshoming (sensorlesshoming example coming soon)

## PCB: ##
PCB is a 4 layer 1.6mm custom PCB, this is what allows it to have its comapact form factor. The bare prototype boards were kindly provided by [PCBWay](https://pcbway.com/g/heg1oh). Schematic and GERBERs can be found in the PCB folder.

## Housing: ##
The top cover can be 3D printed however this may not handle well if the motor & controller get warm under high load. A cover machined from polycarbonite or acrylic is a much better option (and looks awesome). An aluminium housing gives a sleek industrial look as well as acting as a heatsink, however this should not be used if you are using WiFi on the ESP32-S3 as it may block the antenna and reduce the WiFi signal.
![Screenshot 2024-05-23 213000](https://github.com/joshr120/USB-PD-Stepper-Driver/assets/120012174/0c35d216-5a75-4f9d-85c3-fbf170c61fe6)

## Cooling: ##
Stepper motor drivers can get warm under full load. The IC is cooled from the rear through a machined aluminium heatspreader which also acts as a spacer between the PCB and the motor. Two sizes of adhesive heatsinks can also stick to the top side with the larger one protruding through the housing and the smaller one siting just below the surface allowing the stepper to sit flat.

## Software: ##
The software is currently still improving but current sample code should be more than enough to get you started

### ESPHome ###
The provided .yaml file allows you to control the motor through ESPHOME (setup as blinds in this example). It allows you to use the buttons as inputs and set the USB PD voltage. It also outputs the AS5600 encoder reading to be used within ESPHome. The USB PD voltage and stepper microsteps can be set at startup.

### Arduino Code ###
A few arduino examples are provided in this repo:
 - **Simple button control:**
   
      This example uses the buttons on the side to run the motor as at a variety of speeds as well as requesting a specified voltage from the PD controller. This can be used for a simple turntable and uses the simple step and direction interface to the driver

 - **Slider Webpage Control:**
   
      In this example the ESP32 hosts a webpage which allows you to wirelessly control the stepper motors speed with a slider. This utilises the [TMC2209 library](https://github.com/janelia-arduino/TMC2209) to configure the stepper driver.

- **ESP-NOW Sender and Receiver:**

     In this example one PD stepper board wirelessly sends its encoder value to another board which will mimic its position. This is an example of using ESP-NOW for low latency wireless communication.

  More info on the software can be found on the [software page](Software/README.md).

## Control ##
The primary way to control the PD Stepper is over WiFi (e.g ESPHome, ESP NOW or through a webserver) however there are many other ways you can control it

- Thanks to the pin mux on the ESP32-S3 the AUX connector can be used to send/receive commands over many different protocols. For example: Serial UART, I2C and standard STEP & DIR signals.
- By Using the I2C connector as well as the AUX connector you could daisy chain many PD Stepper boards together.
- The standard Qwiic / Stemma QT I2C connector means you can connect a wide range of sensors which could be used for control (for example a [Rotary Encoder](https://www.adafruit.com/product/5880) or a [Light Sensor](https://www.adafruit.com/product/4162))

- The 3 buttons on the side of the board can be set to control speed and/or position

- Serial commands can be sent/received through the USB-C connector (Note if your device sending the commands cannot provide enough power you may need an injector of some kind)

- The ESP32-S3 also has built in BLE (Bluetooth Low Energy) which could be used for control.

## Setting the driver voltage ##
The USB PD voltage can be set by toggling 3 GPIO pins on the ESP32. This can be configured on the fly to change voltage.

By default the board will ask for 20V at startup if the microcontroller does not set the config pins fast enough. If for whatever reason your motor cannot handle 20V you may want to set resistor R18 to a set value or ensure the pins are set on startup. The voltage can be configed by software on the fly as required. 

When programming the board it is likely your computer will only be able to provide 5V, the "Power Good" line from the PD controller can tell the ESP32 it is not receiving the voltage it is asking for and in this case you should proably not enable the stepper driver (as done in the example code)

NOTE: You must use a USB PD power brick/power bank which can provide the required voltage and current along with a suitable USB-C cable. QC (Quick Charge) is not currently supported.

## Setting the driver Current ##
The motor current can be set either in hardware with the onbaord potentiometer or over the TMC2209 serial interface (see the [TMC2209 library](https://github.com/janelia-arduino/TMC2209) for for info on serial comms)
