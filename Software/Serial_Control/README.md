# PD Stepper: Serial Control Guide

Upload the Serial_Control.ino script to control your PD Stepper over serial.

## Connection Options

The controller supports dual-input serial communication. You can send commands through either interface; the controller will process the command and respond back on the same port it was received.

### 1. USB Connection (`Serial`)
* **Physical Port:** Standard USB-C connector on the ESP32-S3.
* **Baud Rate:** 115200

### 2. AUX Connector (`Serial1`)
* **Physical Port:** AUX connector (3 pin JST SH connector).
* **Pins:**
    * **Pin 1 (GND):** Ground pin.
    * **Pin 2 (AUX1 - TX):** Transmits data from the ESP32.
    * **Pin 3 (AUX2 - RX):** Receives data into the ESP32.
* **Baud Rate:** 115200

---

## Control Modes & Switching

The controller operates in a "last command wins" state machine. It switches between modes automatically based on the command received:

* **Position Mode:** Triggered by the `deg=` command. The motor will calculate the distance to the target angle and move using the speed defined by `speed=`.
* **Velocity Mode:** Triggered by the `vel=` command. This immediately overrides any active position targets and causes the motor to spin continuously at the requested rate.

> **Note:** Sending `vel=0` will stop the motor but keep it in Velocity Mode. To return to a specific angle, you must send a new `deg=` command.

---

## Encoder Integration (`MOVE_FROM_ENC`)

When `closed_loop_type` is set to `MOVE_FROM_ENC`, the controller uses the AS5600 Magnetic Encoder to verify its physical orientation before moving.

1.  **Read:** It captures the current mechanical angle from the encoder.
2.  **Calculate:** It determines the error (delta) between the current encoder position and your `deg=` setpoint.
3.  **Correct:** It generates the exact number of microsteps required to bridge that gap.

This allows the system to remain accurate even if the motor was moved by hand while the driver was disabled.

---

## Full Command List

| Command | Data Type | Description |
| :--- | :--- | :--- |
| `deg=` | Float | **Switch to Position Mode:** Move to absolute angle. |
| `deg_rel=` | Float | **Switch to Position Mode:** Relative move to absolute angle. (add or subtracts from current setpoint) |
| `speed=` | Float | Set travel speed for `deg=` movements (deg/s). |
| `vel=` | Float | **Switch to Velocity Mode:** Spin at fixed rate (deg/s). |
| `steps_per_rev=` | Int | Set motor physical steps (200 or 400). |
| `enable=` | Bool/Str | Enable/Disable driver (`1`/`0`, `TRUE`/`FALSE`, `enabled`/`disabled`). |
| `microsteps=` | Int | 1, 4, 8, 16, 32, 64, 128, or 256. |
| `voltage=` | Int | USB PD Request (5, 9, 12, 15, or 20). |
| `current=` | Int | Set run current as a percentage (0-100). |
| `standstill_mode=`| String | `NORMAL`, `FREEWHEELING`, `BRAKING`, or `STRONG_BRAKING`. |
| `closed_loop_type=`| String | `OPEN_LOOP` or `MOVE_FROM_ENC`. |
| `mappingDirection=`| Int | Reverse motor logic relative to encoder (`0` or `1`). |
| `verboseOutput=` | Int | Toggle confirmation messages (`0` or `1`). |
| `get_angle` | N/A | Returns current total encoder angle in degrees. |
| `values` | N/A | Lists all current configuration settings. |
| `led_flash` | N/A | Flashes LED1 twice for identification. |
| `help` | N/A | Displays summary of commands. |
