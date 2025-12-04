# **PD Stepper** - Getting Started
If you just received your PD Stepper this will show you how to spin a motor and configure the driver using the firmware pre-installed on the board.

⚠️ **Warning:** Stepper motors should never be back driven at a high speed as they can act as a generator and cause damage. This is especially important if your PD stepper board is connected directly to a computers USB port

## Assembling a kit onto a Nema 17 ##

Timestamp 1:50 of [this video](https://youtu.be/qECEGUZE04s?t=110) also shows how to setup a Nema 17 with your kit.

### 1. Install the magnet onto your motor (only needed for reading encoder position)
   
   Carefully superglue the included diametrically magnitised magnet onto the centre of the motor shaft.  

   Only a very small amount of glue is required, take care not to get any in the motor bearing.  
   
   <img src="https://github.com/user-attachments/assets/5f30bf37-0826-4acc-aae3-19e3e8c6ea71" width="40%" style="margin: 15px;"/>
   
### 2. Remove the original motor screws

<img src="https://github.com/user-attachments/assets/3d0ed5a5-f627-423b-a659-64f82d3efd02" width="40%" />
<img src="https://github.com/user-attachments/assets/6c356f4e-3def-484e-ba32-874d9b31d21a" width="40%" />

### 3. Peel off the protective film and add one of the included heat pads onto the heat spreader

  Ensure this is added on the recessed side of the heat spreader
  
<img src="https://github.com/user-attachments/assets/4cc5557a-db9c-483b-81b6-23050e741b87" width="40%" />
<img src="https://github.com/user-attachments/assets/488c5d0f-4e0d-451d-9b88-532666b6c4e1" width="40%" />

### 4. Place the spacer onto the motor ensuring it is in the orinentaion show

Note. If you are not using the heatspreader/spacer (partial kit) ensure the board is not resting directly on the motor as this could short out the traces on the underside of the PCB. Use washers or an insualting layer.

<img src="https://github.com/user-attachments/assets/8f998eb9-08c3-423f-9fc1-47906d7a12e5" width="40%" />

### 5. Add the PCB

Again ensure the PCB is installed in the orientation shown so the motor connectors as well as the heat pad line up

<img src="https://github.com/user-attachments/assets/422a96af-25d4-471b-8eb3-9a04f6864500" width="40%" />
<img src="https://github.com/user-attachments/assets/50db331a-7b4e-4453-b08a-cd70c9577580" width="40%" />

### 5. Add the cover and new countersunk screws

   The included screws are best suited for a 34mm length Nema 17 you may need longer or shorter ones depending on your motor. (included ones are M3x40mm)
   
   <img src="https://github.com/user-attachments/assets/25896c30-5707-4bac-831d-a3beff13babb" width="40%" />
   <img src="https://github.com/user-attachments/assets/a0a7c2d4-a4b8-4c05-8bea-e5524033c756" width="40%" />

### 6. Add the heatsink

   Use the larger heatsink if you plan on running the motor for long periods and/or at high currents
   
   <img src="https://github.com/user-attachments/assets/c09d56a1-1f89-4c6e-957b-8700879945e3" width="40%" />
   <img src="https://github.com/user-attachments/assets/b4dd616e-1a98-4409-8d25-f4f41b0ff4a9" width="40%" />

### 7. Plug in motor loom

 There are 2 looms provided in the kit for different motors, try one and if at the next step your motor stutters/doesnt spin, power off the board and try the other one. 

  Note. Do not disconnect/connect the motor while the board is powered.

  <img src="https://github.com/user-attachments/assets/fcf5dcf0-d262-47ac-b3af-f18d4d0da2c7" width="50%" />

<br>
<br>

## Running the Webserver ##
**1.** Connect the PD Stepper to your PD power supply using a USB C to C cable (Needs to be Type C as USB PD does not support Type A).

**2.** Open a phone/computer and connect to the WiFi network named "PD Stepper"

**3.** Open a browser and type in the IP Address **192.168.4.1**

<img src="https://github.com/user-attachments/assets/c4efd506-1c79-46b7-9adb-9545b20af1c9" width="40%" />
   
**5.** From here you can use velocity/postion control and play around with different settings.

**6.** The current is set as a percentage of the maximum possible current (1.92A). You will want to set this to match the current rating of your motor (or lower to stay cooler) E.g for a 1.8A motor set to 90% as 90% * 1.92A = 1.73A

Note: When using the webserver example the value is set here and the potentiometer is ignored. When using the advanced ESPHome examples this can be set with by changing the `run_current` value.

**7.** For your own projects this code as well as others (such as ESPHome config) can be found [HERE](/Software)

**8.** The buttons on the side can also be used to change the velocity

## Trouble Shooting ##
- The motor will not spin unless the board is receiving correct the voltage requested from the PD Source. So ensure your brick can supply the correct voltage. The requested voltage can be adjusted on the fly in the webserver.
