# **PD Stepper** - Getting Started
If you just received your PD Stepper this will show you how to spin a motor and configure the driver using the firmware pre-installed on the board.

## Connect Your Stepper Motor ##

Timestamp 1:50 of [this video](https://youtu.be/qECEGUZE04s?t=110) shows how to setup a Nema 17 with your kit. There are 2 looms provided in the kit for different motors, try one and if at the next step your motor stutters/doesnt spin, power off the board and try the other one. 

NOTES: 
1. Do not disconnect/connect the motor while the board is powered.
2. If you are not using the heatspreader/spacer (partial kit) ensure the board is not resting directly on the motor as this could short out the traces on the underside of the PCB. Use washers or an insualting layer.

## Running the Webserver ##
**1.** Connect the PD Stepper to your PD power supply.

**2.** Open a phone/computer and connect to the WiFi network named "PD Stepper"

**3.** Open a browser and type in the IP Address **192.168.4.1**

<img src="https://github.com/user-attachments/assets/c4efd506-1c79-46b7-9adb-9545b20af1c9" width="40%" />
   
**5.** From here you can use velocity/postion control and play around with different settings.

**6.** For your own projects this code as well as others (such as ESPHome config) can be found [HERE](/Software)

## Trouble Shooting ##
- The motor will not spin unless the board is receiving correct the voltage requested from the PD Source. So ensure your brick can supply the correct voltage. The requested voltage can be adjusted on the fly in the webserver.
