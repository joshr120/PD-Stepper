/*
 * PD Stepper Serial Control Example
 * * Modified to support Serial commands on USB (Serial) 
 * and AUX1/AUX2 pins (Serial1).
 */

#include <TMC2209.h> //  https://github.com/janelia-arduino/TMC2209/tree/main
#include <Preferences.h> //for saving to flash (instead of old EEPROM lib)
#include <Wire.h> //For I2C for encoder

Preferences preferences;

//TMC2209 setup
TMC2209 stepper_driver;
HardwareSerial & serial_stream = Serial2;
const long SERIAL_BAUD_RATE = 115200;

const uint8_t RUN_CURRENT_PERCENT = 20; //how much current to run at (0-100%)

//TMC2209 Stepper Driver
#define TMC_EN  21
#define STEP    5
#define DIR     6
#define MS1     1
#define MS2     2
#define SPREAD  7
#define TMC_TX  17
#define TMC_RX  18
#define DIAG    16
#define INDEX   11

//PD Trigger (CH224K)
#define PG      15  //power good singnal (dont enable stepper untill this is good)
#define CFG1    38
#define CFG2    48
#define CFG3    47

//Other
#define VBUS    4
#define NTC     7
#define LED1    10
#define LED2    12
#define SW1     35
#define SW2     36
#define SW3     37
#define AUX1    14
#define AUX2    13

//Global variables
int set_speed = 0;
bool PGState = 0; 
bool enabledState = 0;
bool state = 0; 

//AS5600 Hall Effect Encoder
#define AS5600_ADDRESS 0x36 
signed long total_encoder_counts = 0;
double angle = 0; 
int encoder_offset = 0; 

unsigned long lastEncRead = 0;
int mainFreq = 50; 

//button read and debounce
bool incButtonState = HIGH;
bool decButtonState = HIGH;
bool resetButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int buttonSpeed = 0;

//Voltage reading and calc
float VBusVoltage = 0;
float VREF = 3.3;
const float DIV_RATIO = 0.1189427313;  

//Default values
bool enabled1 = 1;
int setVoltage = 12;
int microsteps = 64;
int current = 20;
int stallThreshold = 10;
String standstillMode = "NORMAL";
float positionSpeed = 360;
int stepsPerRev = 200;
String closedLoopType = "MOVE_FROM_ENC";
bool mappingDirection = 1;
bool verboseOutput = 1;

//Variables for position control
signed long setPoint = 0;
signed long CurrentPosition = 0;
unsigned long lastStep = 0;
signed long microsteps_to_move = 0;

// Forward Declarations
void readSerialCommands(Stream &port);
unsigned long getStepDelay(float deg_per_sec, int steps_per_rev, int microsteps);
void readEncoder();
void configureSettings();
void readSettings();
void writeSettings();

void setup() {
  //PD Trigger Setup
  pinMode(PG, INPUT);
  pinMode(CFG1, OUTPUT);
  pinMode(CFG2, OUTPUT);
  pinMode(CFG3, OUTPUT);
  digitalWrite(CFG1, LOW);  
  digitalWrite(CFG2, LOW);  
  digitalWrite(CFG3, HIGH); 

  //General
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);

  //Setup serial comms with TMC2209
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(TMC_EN, OUTPUT);
  pinMode(DIAG, INPUT);
  digitalWrite(TMC_EN, LOW); 

  digitalWrite(MS1, LOW); 
  digitalWrite(MS2, LOW);

  //AS5600 Hall Encoder Setup
  Wire.begin(SDA, SCL);  

  readSettings(); 
  
  stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE, TMC2209::SERIAL_ADDRESS_0, TMC_RX, TMC_TX);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.enableAutomaticCurrentScaling(); 
  stepper_driver.enableStealthChop(); 
  stepper_driver.setCoolStepDurationThreshold(5000); 
  stepper_driver.disable();

  configureSettings(); 

  readEncoder();
  encoder_offset = total_encoder_counts;

  delay(200); 
  
  // USB Serial
  Serial.begin(115200);
  
  // AUX Serial (Serial1) on AUX1 (TX) and AUX2 (RX)
  // Note: Ensure correct TX/RX logic for your external device
  Serial1.begin(115200, SERIAL_8N1, AUX2, AUX1); 

  digitalWrite(LED1, HIGH); 
  delay(200);
  digitalWrite(LED1, LOW);  
}

void loop() {
  // Check USB Serial
  if (Serial.available()) {
    readSerialCommands(Serial);
  }

  // Check AUX Serial
  if (Serial1.available()) {
    readSerialCommands(Serial1);
  }

  if (millis() - lastEncRead >= mainFreq){ 
    lastEncRead = millis();
    readEncoder(); 

    digitalWrite(LED2, digitalRead(DIAG)); 

    PGState = digitalRead(PG); 
    if (PGState == LOW and enabled1 == 1 and enabledState == 0){ 
      stepper_driver.enable();
      enabledState = 1;
    } else if ((PGState == HIGH or enabled1 == 0) and enabledState == 1){ 
      stepper_driver.disable();
      enabledState = 0;
    }
  }

  unsigned long step_delay_us = getStepDelay(positionSpeed, stepsPerRev, microsteps); 
 
  if ((micros()-lastStep > (step_delay_us/2)) and (state == HIGH)){
    state = LOW;
    digitalWrite(STEP, LOW);
  }

  // OPEN LOOP CONTROL
  if (closedLoopType == "OPEN_LOOP"){
    if (setPoint > (CurrentPosition + 256/microsteps)){
      if (micros()-lastStep > step_delay_us){
        digitalWrite(DIR, mappingDirection ? HIGH : LOW);
        state = HIGH;
        digitalWrite(STEP, HIGH);
        CurrentPosition = CurrentPosition + (256/microsteps);  
        lastStep = micros();
      }
    } else if (setPoint < (CurrentPosition - 256/microsteps)){
      if (micros()-lastStep > step_delay_us){
        digitalWrite(DIR, mappingDirection ? LOW : HIGH);
        state = HIGH;
        digitalWrite(STEP, HIGH);
        CurrentPosition = CurrentPosition - (256/microsteps);  
        lastStep = micros();
      }
    }
  } else {
    // MOVE FROM ENCODER
    if (microsteps_to_move >= 256/microsteps){
      if (micros()-lastStep > step_delay_us){
        digitalWrite(DIR, mappingDirection ? HIGH : LOW);
        state = HIGH;
        digitalWrite(STEP, HIGH);
        microsteps_to_move = microsteps_to_move - (256/microsteps);  
        lastStep = micros();
      }
    } else if (microsteps_to_move <= -256/microsteps){
      if (micros()-lastStep > step_delay_us){
        digitalWrite(DIR, mappingDirection ? LOW : HIGH);
        state = HIGH;
        digitalWrite(STEP, HIGH);
        microsteps_to_move = microsteps_to_move + (256/microsteps);  
        lastStep = micros();
      }
    }
  }

  // Button handling
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    bool currentIncButtonState = digitalRead(SW3);
    bool currentDecButtonState = digitalRead(SW1);
    bool currentResetButtonState = digitalRead(SW2);
  
    if (currentIncButtonState != incButtonState) {
      incButtonState = currentIncButtonState;
      if (incButtonState == LOW) {
        buttonSpeed = min(buttonSpeed + 10, 330);
        stepper_driver.moveAtVelocity(buttonSpeed*(microsteps));
      }
    }
    if (currentDecButtonState != decButtonState) {
      decButtonState = currentDecButtonState;
      if (decButtonState == LOW) {
        buttonSpeed = max(buttonSpeed - 10, -330);
        stepper_driver.moveAtVelocity(buttonSpeed*(microsteps));
      }
    }
    if (currentResetButtonState != resetButtonState) {
      resetButtonState = currentResetButtonState;
      if (resetButtonState == LOW) {
        buttonSpeed = 0;
        stepper_driver.moveAtVelocity(0);
      }
    }
  }
}

void readSerialCommands(Stream &port) {
  String cmd = port.readStringUntil('\n');
  cmd.trim();

  if (cmd.startsWith("deg=")) {
    float targetAngle = cmd.substring(4).toFloat();
    setPoint = (targetAngle/360.0) * stepsPerRev * 256; 
    signed long posMicrosteps = (stepsPerRev * 256 * (total_encoder_counts-encoder_offset)) / 4096.0;
    microsteps_to_move = setPoint - posMicrosteps; 
    if (verboseOutput) {
      port.print("SUCCESS: Target angle set to: ");
      port.println(targetAngle);
    }
  }
  else if (cmd.startsWith("vel=")) {
    float motorSpeed = cmd.substring(4).toFloat();
    int32_t microsteps_per_period = ((motorSpeed/360.0)*stepsPerRev*microsteps)/0.715;
    stepper_driver.moveAtVelocity(microsteps_per_period*-1);
    if (verboseOutput) {
      port.print("SUCCESS: Speed set to: ");
      port.println(motorSpeed);
    }
  }
  else if (cmd.startsWith("steps_per_rev=")) {
    int stepsPerRevRec = cmd.substring(14).toInt();
    if (stepsPerRevRec == 200 or stepsPerRevRec == 400){
      stepsPerRev = stepsPerRevRec;
      if (verboseOutput) {
        port.print("SUCCESS: Steps per revolution set to: ");
        port.println(stepsPerRev);
      }
      writeSettings();
    } else {
      port.println("ERROR: Steps per revolution must be 200 or 400 ");
    }
  }
  else if (cmd.startsWith("enable=")) {
    String enableRec = cmd.substring(7);
    if (enableRec == "0" or enableRec == "FALSE" or enableRec == "disabled"){
      enabled1 = 0;
      if (verboseOutput) {port.println("SUCCESS: Driver disabled");}
      writeSettings();
    } else if (enableRec == "1" or enableRec == "TRUE" or enableRec == "enabled"){
      enabled1 = 1;
      if (verboseOutput) {port.println("SUCCESS: Driver enabled");}
      writeSettings();
    } else {
      port.println("ERROR: Enable must be set to 0, 1, FALSE or TRUE");
    }
  }
  else if (cmd.startsWith("microsteps=")) {
    int microstepsRec = cmd.substring(11).toInt();
    if (microstepsRec == 1 || microstepsRec == 4 || microstepsRec == 8 || microstepsRec == 16 || 
        microstepsRec == 32 || microstepsRec == 64 || microstepsRec == 128 || microstepsRec == 256){
      microsteps = microstepsRec;
      writeSettings();
      if (verboseOutput) {
        port.print("SUCCESS: Microsteps set to: ");
        port.println(microsteps);
      }
    } else {
      port.println("ERROR: microsteps must be 1, 4, 8, 16, 32, 64, 128 or 256");
    }
  }
  else if (cmd.startsWith("voltage=")) {
    int setVoltageRec = cmd.substring(8).toInt();
    if (setVoltageRec == 5 || setVoltageRec == 9 || setVoltageRec == 12 || setVoltageRec == 15 || setVoltageRec == 20){
      setVoltage = setVoltageRec;
      if (verboseOutput) {
        port.print("SUCCESS: PD Voltage set to: ");
        port.println(setVoltage);
      }
      writeSettings();
    } else {
      port.println("ERROR: Voltage must be 5, 9, 12, 15 or 20");
    }
  }
  else if (cmd.startsWith("current=")) {
    int currentRec = cmd.substring(8).toInt();
    if (currentRec >= 0 and currentRec <= 100){
      current = currentRec;
      if (verboseOutput) {
        port.print("SUCCESS: Current set to: ");
        port.print(current);
        port.println("%");
      }
      writeSettings();
    } else {
      port.println("ERROR: Current needs to be a value from 0-100");
    }
  }
  else if (cmd.startsWith("speed=")) {
    float speedRec = cmd.substring(6).toFloat();
    if (speedRec >= 0 and speedRec <= 5000){
      positionSpeed = speedRec;
      if (verboseOutput) {
        port.print("SUCCESS: Position speed value set to: ");
        port.println(positionSpeed);
      }
      writeSettings();
    } else {
      port.println("ERROR: Position speed must be between 0 and 5000 deg/s");
    }
  }
  else if (cmd.startsWith("standstill_mode=")) {
    String standstillModeRec = cmd.substring(16);
    if (standstillModeRec == "NORMAL" || standstillModeRec == "FREEWHEELING" || 
        standstillModeRec == "BRAKING" || standstillModeRec == "STRONG_BRAKING"){
      standstillMode = standstillModeRec;
      if (verboseOutput) {
        port.print("SUCCESS: Standstill mode set to: ");
        port.println(standstillMode);
      }
      writeSettings();
    } else {
      port.println("ERROR: Standstill mode must be NORMAL, FREEWHEELING, BRAKING or STRONG_BRAKING");
    }
  }
  else if (cmd.startsWith("closed_loop_type=")) {
    String closedLoopTypeRec = cmd.substring(17);
    if (closedLoopTypeRec == "OPEN_LOOP" || closedLoopTypeRec == "MOVE_FROM_ENC" || closedLoopTypeRec == "CLOSED_LOOP"){
      closedLoopType = closedLoopTypeRec;
      if (verboseOutput) {
        port.print("SUCCESS: Closed loop mode set to: ");
        port.println(closedLoopType);
      }
      writeSettings();
    } else {
      port.println("ERROR: MOVE_FROM_ENC must be OPEN_LOOP, MOVE_FROM_ENC Or CLOSED_LOOP");
    }
  }
  else if (cmd.startsWith("mappingDirection=")) {
    int mappingDirectionRec = cmd.substring(17).toInt();
    if (mappingDirectionRec == 0 or mappingDirectionRec == 1){
      mappingDirection = mappingDirectionRec;
      if (verboseOutput) {
        port.print("SUCCESS: Mapping direction mode set to: ");
        port.println(mappingDirection);
      }
      writeSettings();
    } else {
      port.println("ERROR: Mapping direction must be 0 or 1");
    }
  }
  else if (cmd.startsWith("verboseOutput=")) {
    String verboseOutputRec = cmd.substring(14);
    if (verboseOutputRec == "0" or verboseOutputRec == "FALSE" or verboseOutputRec == "disabled"){
      verboseOutput = 0;
      writeSettings();
    } else if (verboseOutputRec == "1" or verboseOutputRec == "TRUE" or verboseOutputRec == "enabled"){
      verboseOutput = 1;
      port.println("SUCCESS: verbose output enabled");
      writeSettings();
    } else {
      port.println("ERROR: verbose output must be set to 0, 1, FALSE or TRUE");
    }
  }
  else if (cmd.equalsIgnoreCase("HELP")) {
    port.println("\nList of available commands:");
    port.println("deg= xxx.x          : move to position, in degrees");
    port.println("vel= xxxx           : spin at set velocity");
    port.println("steps_per_rev= xxx  : Steps per revolution, 200 or 400");
    port.println("enable= x           : enable/disable driver, 0 or 1");
    port.println("microsteps= xxx     : set microsteps, 1, 4, 8, 16, 32, 64, 128, 256");
    port.println("voltage= xx         : set USB PD Voltage, 5, 9, 12, 15 or 20");
    port.println("current= xxx        : set driver current %, 0-100");
    port.println("speed= xxxx         : set position speed deg/s, 0-5000");
    port.println("standstill_mode= x  : NORMAL, FREEWHEELING, BRAKING, STRONG_BRAKING");
    port.println("closed_loop_type=x  : OPEN_LOOP, MOVE_FROM_ENC, CLOSED_LOOP");
    port.println("mappingDirection=x  : reverse direction, 0 or 1");
    port.println("verboseOutput=x     : 0 or 1");
    port.println("values              : output current setup");
    port.println("led_flash           : flash LED1 twice");
    port.println("get_angle           : return current angle in degrees\n");
  }
  else if (cmd.equalsIgnoreCase("VALUES")) {
    port.println("\nCurrent Driver Setup:");
    port.print("steps_per_rev = "); port.println(stepsPerRev);
    port.print("enable = "); port.println(enabled1);
    port.print("microsteps = "); port.println(microsteps);
    port.print("voltage = "); port.println(setVoltage);
    port.print("current = "); port.println(current);
    port.print("speed = "); port.println(positionSpeed);
    port.print("standstill_mode = "); port.println(standstillMode);
    port.print("closed_loop_type = "); port.println(closedLoopType);
    port.print("mappingDirection = "); port.println(mappingDirection);
    port.print("verboseOutput = "); port.println(verboseOutput);
    port.println("");
  }
  else if (cmd.equalsIgnoreCase("led_flash")) {
    digitalWrite(LED1, HIGH); delay(300); digitalWrite(LED1, LOW);
    delay(200);
    digitalWrite(LED1, HIGH); delay(300); digitalWrite(LED1, LOW);
  }
  else if (cmd.equalsIgnoreCase("get_angle")) {
    port.println(angle);
  }
  else {
    port.println("ERROR: Unknown command, enter 'help' for a list of commands");
  }
}

unsigned long getStepDelay(float deg_per_sec, int steps_per_rev, int microsteps) {
  double total_steps_per_rev = steps_per_rev * microsteps;
  double steps_per_sec = (deg_per_sec / 360.0) * total_steps_per_rev;
  if (steps_per_sec <= 0) return 0;
  return (unsigned long)((1.0 / steps_per_sec) * 1e6);
}

void readEncoder(){
  int raw_counts;
  static int prev_raw_counts = 0;
  static signed long revolutions = 0;
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(0x0C); 
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDRESS, 2);
  if (Wire.available() >= 2) {
    raw_counts = Wire.read() << 8 | Wire.read();
  }
  if (prev_raw_counts > 3000 && raw_counts < 1000) {
    revolutions++;
  } else if (prev_raw_counts < 1000 && raw_counts > 3000) {
    revolutions--;
  }
  prev_raw_counts = raw_counts;
  total_encoder_counts = raw_counts + (4096 * revolutions);
  angle = ((total_encoder_counts-encoder_offset) * 360.0) / 4096.0;
}

void configureSettings(){
  if (setVoltage == 5)       { digitalWrite(CFG1, HIGH); } 
  else if (setVoltage == 9)  { digitalWrite(CFG1, LOW); digitalWrite(CFG2, LOW); digitalWrite(CFG3, LOW); } 
  else if (setVoltage == 12) { digitalWrite(CFG1, LOW); digitalWrite(CFG2, LOW); digitalWrite(CFG3, HIGH); } 
  else if (setVoltage == 15) { digitalWrite(CFG1, LOW); digitalWrite(CFG2, HIGH); digitalWrite(CFG3, HIGH); } 
  else if (setVoltage == 20) { digitalWrite(CFG1, LOW); digitalWrite(CFG2, HIGH); digitalWrite(CFG3, LOW); }

  stepper_driver.setRunCurrent(current);
  stepper_driver.setMicrostepsPerStep(microsteps);
  stepper_driver.setStallGuardThreshold(stallThreshold);

  if (standstillMode == "NORMAL") stepper_driver.setStandstillMode(stepper_driver.NORMAL);
  else if (standstillMode == "FREEWHEELING") stepper_driver.setStandstillMode(stepper_driver.FREEWHEELING);
  else if (standstillMode == "BRAKING") stepper_driver.setStandstillMode(stepper_driver.BRAKING);
  else if (standstillMode == "STRONG_BRAKING") stepper_driver.setStandstillMode(stepper_driver.STRONG_BRAKING);
}

void readSettings(){ 
  preferences.begin("settings", false);
  enabled1 = preferences.getBool("enable", enabled1); 
  setVoltage = preferences.getInt("voltage", setVoltage); 
  microsteps = preferences.getInt("microsteps", microsteps); 
  current = preferences.getInt("current", current); 
  stallThreshold = preferences.getInt("stallThreshold", stallThreshold); 
  standstillMode = preferences.getString("standstillMode", standstillMode); 
  positionSpeed = preferences.getFloat("positionSpeed", positionSpeed); 
  stepsPerRev = preferences.getInt("stepsPerRev", stepsPerRev); 
  mappingDirection = preferences.getBool("mapDir", mappingDirection); 
  closedLoopType = preferences.getString("closedLoopType", closedLoopType); 
  verboseOutput = preferences.getBool("verboseOutput", verboseOutput); 
  preferences.end();
}

void writeSettings(){
  preferences.begin("settings", false);
  preferences.putBool("enable", enabled1); 
  preferences.putInt("voltage", setVoltage); 
  preferences.putInt("microsteps", microsteps); 
  preferences.putInt("current", current); 
  preferences.putInt("stallThreshold", stallThreshold); 
  preferences.putString("standstillMode", standstillMode); 
  preferences.putFloat("positionSpeed", positionSpeed); 
  preferences.putInt("stepsPerRev", stepsPerRev); 
  preferences.putBool("mapDir", mappingDirection); 
  preferences.putString("closedLoopType", closedLoopType); 
  preferences.putBool("verboseOutput", verboseOutput); 
  preferences.end();
  configureSettings();
}