/*
 * PD Stepper Serial Control Example
 * 
 * TODO:
 *  - Option for continuus encoder output (choose Hz)
 *  - Option to change encoder type (counts or deg)
 *  - Full info out (GPIO READ, PD voltage, stall state, etc)?
 *  - Stallguard output / threshold set
 *  - Add acceleration/decelleration
 *  - Closed loop control and/or Velocity Feedforward + Position Feedback Correction Loop
*/


#include <TMC2209.h> //  https://github.com/janelia-arduino/TMC2209/tree/main

#include <Preferences.h> //for saving to flash (instead of old EEPROM lib)

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
bool PGState = 0; //state of the power good signal from PD sink IC
bool enabledState = 0;
bool state = 0; //step state


//AS5600 Hall Effect Encoder
#include <Wire.h> //For I2C for encoder
#define AS5600_ADDRESS 0x36 // I2C address of the AS5600 sensor
signed long total_encoder_counts = 0;
double angle = 0; //total encoder angle in deg
int encoder_offset = 0; //initial enocder value


unsigned long lastEncRead = 0;
int mainFreq = 50; //Scheduled frequency = 100hz (for slower tasks, encoder reading etc)


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
const float DIV_RATIO = 0.1189427313;  //20k&2.7K Voltage Divider

//Default values stored and updated from EEPROM
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

//Varaiables for position control (open loop)
signed long setPoint = 0;
signed long CurrentPosition = 0;
unsigned long lastStep = 0;

signed long microsteps_to_move = 0;

void setup() {
   //PD Trigger Setup
  pinMode(PG, INPUT);
  pinMode(CFG1, OUTPUT);
  pinMode(CFG2, OUTPUT);
  pinMode(CFG3, OUTPUT);
                            //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
  digitalWrite(CFG1, LOW);  //  1    0     0     0     0
  digitalWrite(CFG2, LOW);  //  -    0     0     1     1
  digitalWrite(CFG3, HIGH); //  -    0     1     1     0

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
  pinMode(MS1, OUTPUT);
  pinMode(TMC_EN, OUTPUT);
  pinMode(DIAG, INPUT);
  digitalWrite(TMC_EN, LOW); //Enabled here and later enabled/disabled over UART

  digitalWrite(MS1, LOW); //used to set serial address in UART mode
  digitalWrite(MS2, LOW);

  //AS5600 Hall Encoder Setup
  Wire.begin(SDA, SCL);  //start wire with earlier defined pins


  readSettings(); //get saved values from EEPROM
  
  stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE, TMC2209::SERIAL_ADDRESS_0, TMC_RX, TMC_TX);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.enableAutomaticCurrentScaling(); //current control mode
//  stepper_driver.enableCoolStep();
  stepper_driver.enableStealthChop(); //stealth chop needs to be enabled for stall detect
  stepper_driver.setCoolStepDurationThreshold(5000); //TCOOLTHRS (DIAG only enabled when TSTEP smaller than this)
  stepper_driver.disable();

  configureSettings(); //use saved settings

  //get encoder offset
  readEncoder();
  encoder_offset = total_encoder_counts;

  delay(200); //delay needed before "Serial.begin" to ensure bootloader mode entered correctly. Otherwise bootloader mode may need to be manually entered by holding BOOT, press RST, release BOOT
  Serial.begin(115200);
//  Serial.println("Code Starting");

  digitalWrite(LED1, HIGH); //flash LED after setep complete
  delay(200);
  digitalWrite(LED1, LOW); 

}

void loop() {

  readSerialCommands(); //receive commands (may need to be scheduled)
  

  if (millis() - lastEncRead >= mainFreq){ //main loop
    lastEncRead = millis();
    readEncoder(); //need to constantly read encoder in order to catch wrap around

    digitalWrite(LED2, digitalRead(DIAG)); //Stall detection state output to LED

    //Disable motor if PD "Power Good" not good
    PGState = digitalRead(PG); //read state, LOW = enabled
//    PGState = 0; /////////  OVERRIDE /////// uncomment to override
    if (PGState == LOW and enabled1 == 1 and enabledState == 0){ //only enable if PD chip reports good
      stepper_driver.enable();
      enabledState = 1;
    } else if ((PGState == HIGH or enabled1 == 0) and enabledState == 1){ //Disable stepper if box unchecked or PG is high
      stepper_driver.disable();
      enabledState = 0;
    }
  }


  
 //get delay based on deg/s, microsteps, steps pre rev
 unsigned long step_delay_us = getStepDelay(positionSpeed, stepsPerRev, microsteps); //can probably move this, dont need to re-calc each time
 
  //set STEP output low after half the delay as passed 
  if ((micros()-lastStep > (step_delay_us/2)) and (state == HIGH)){
    state = LOW;
    digitalWrite(STEP, LOW);
  }

  //OPEN LOOP CONTROL
  if (closedLoopType == "OPEN_LOOP"){
    //move to setpoint using delay adjusted for current micostep value
    if (setPoint > (CurrentPosition + 256/microsteps)){ //position control (move if setpoint more than 1 microstep away)
      if (micros()-lastStep > step_delay_us){ //always move same speed regardless of what microsteps set
        digitalWrite(DIR, mappingDirection ? HIGH : LOW); //set direction based on encoder mapping dir (as some motors wired in reverse)
        state = HIGH;
        digitalWrite(STEP, HIGH);
        CurrentPosition = CurrentPosition + (256/microsteps);  //update current position taking into account microsteps set
        lastStep = micros();
      }
      
    } else if (setPoint < (CurrentPosition - 256/microsteps)){ //position control (move if setpoint more than 1 microstep away)
      if (micros()-lastStep > step_delay_us){
        digitalWrite(DIR, mappingDirection ? LOW : HIGH); //set direction based on encoder mapping dir (as some motors wired in reverse)
        state = HIGH;
        digitalWrite(STEP, HIGH);
        CurrentPosition = CurrentPosition - (256/microsteps);  //update current position taking into account microsteps set
        lastStep = micros();
      }
    }

  //MOVE FROM ENCODER POSITION control 
  } else {
    //move to setpoint using delay adjusted for current micostep value
    if (microsteps_to_move >= 256/microsteps){ //position control (move if setpoint more than 1 microstep away)
      if (micros()-lastStep > step_delay_us){ //always move same speed regardless of what microsteps set
        digitalWrite(DIR, mappingDirection ? HIGH : LOW); //set direction based on encoder mapping dir (as some motors wired in reverse)
        state = HIGH;
        digitalWrite(STEP, HIGH);
        microsteps_to_move = microsteps_to_move - (256/microsteps);  //update number of microsteps left to move
        lastStep = micros();
      }
      
    } else if (microsteps_to_move <= -256/microsteps){ //position control (move if setpoint more than 1 microstep away)
      if (micros()-lastStep > step_delay_us){
        digitalWrite(DIR, mappingDirection ? LOW : HIGH); //set direction based on encoder mapping dir (as some motors wired in reverse)n
        state = HIGH;
        digitalWrite(STEP, HIGH);
        microsteps_to_move = microsteps_to_move + (256/microsteps);  //update current position taking into account microsteps set
        lastStep = micros();
      }
    }
  }
  
  

  //Handle buttons inputs (seperate velocity control)
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    bool currentIncButtonState = digitalRead(SW3);
    bool currentDecButtonState = digitalRead(SW1);
    bool currentResetButtonState = digitalRead(SW2);
  
    if (currentIncButtonState != incButtonState) {
      incButtonState = currentIncButtonState;
      if (incButtonState == LOW) {
        buttonSpeed = buttonSpeed + 10;
        if (buttonSpeed > 330){
          buttonSpeed = 330;
        }
        stepper_driver.moveAtVelocity(buttonSpeed*(microsteps));
      }
    }
  
    if (currentDecButtonState != decButtonState) {
      decButtonState = currentDecButtonState;
      if (decButtonState == LOW) {
        buttonSpeed = buttonSpeed -10;
        if (buttonSpeed < -330){
          buttonSpeed = -330;
        }
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

void readSerialCommands() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("deg=")) {
      float targetAngle = cmd.substring(4).toFloat();
      //SET TO POSITION CONTROL MODE HERE????
      setPoint = (targetAngle/360.0) * stepsPerRev * 256; //convert deg to microsteps

      //used for "move from encoder position" control
      signed long posMicrosteps = (stepsPerRev * 256 * (total_encoder_counts-encoder_offset)) / 4096.0; //current position in microsteps
      microsteps_to_move = setPoint - posMicrosteps; //get the number of microsteps to move based on current position

      if (verboseOutput) {
        Serial.print("SUCCESS: Target angle set to: ");
        Serial.println(targetAngle);
      }
    }
    
    else if (cmd.startsWith("vel=")) {
      float motorSpeed = cmd.substring(4).toFloat();

      //convert command from deg/s to microsteps_per_period
      int32_t microsteps_per_period = ((motorSpeed/360.0)*stepsPerRev*microsteps)/0.715;
      
      stepper_driver.moveAtVelocity(microsteps_per_period*-1);  //move at set speed
      if (verboseOutput) {
        Serial.print("SUCCESS: Speed set to: ");
        Serial.println(motorSpeed);
      }
    }
    
    else if (cmd.startsWith("steps_per_rev=")) {
      int stepsPerRevRec = cmd.substring(14).toInt();
      if (stepsPerRevRec == 200 or stepsPerRevRec == 400){
        stepsPerRev = stepsPerRevRec;
        if (verboseOutput) {
          Serial.print("SUCCESS: Steps per revolution set to: ");
          Serial.println(stepsPerRev);
        }
        writeSettings(); //Save to EEPROM
      }
      else {
        Serial.println("ERROR: Steps per revolution must be 200 or 400 ");
      }
    }

    else if (cmd.startsWith("enable=")) {
      String enableRec = cmd.substring(7);
      if (enableRec == "0" or enableRec == "FALSE" or enableRec == "disabled"){
        enabled1 = 0;
        if (verboseOutput) {Serial.println("SUCCESS: Driver disabled");}
        writeSettings(); //Save to EEPROM
      } else if (enableRec == "1" or enableRec == "TRUE" or enableRec == "enabled"){
        enabled1 = 1;
        if (verboseOutput) {Serial.println("SUCCESS: Driver enabled");}
        writeSettings(); //Save to EEPROM
      }
      else {
        Serial.println("ERROR: Enable must be set to 0, 1, FALSE or TRUE");
      }
    }
    
    else if (cmd.startsWith("microsteps=")) {
      int microstepsRec = cmd.substring(11).toInt();
      if (microstepsRec == 1 or microstepsRec == 4 or microstepsRec == 8 or microstepsRec == 16 or microstepsRec == 32 or microstepsRec == 64 or microstepsRec == 128 or microstepsRec == 256){
        microsteps = microstepsRec;
        writeSettings(); //Save to EEPROM
        if (verboseOutput) {
          Serial.print("SUCCESS: Microsteps set to: ");
          Serial.println(microsteps);
        }
      }
      else {
        Serial.println("ERROR: microsteps must be 1, 4, 8, 16, 32, 64, 128 or 256");
      }
    }

    else if (cmd.startsWith("voltage=")) {
      int setVoltageRec = cmd.substring(8).toInt();
      if (setVoltageRec == 5 or setVoltageRec == 9 or setVoltageRec == 12 or setVoltageRec == 15 or setVoltageRec == 20){
        setVoltage = setVoltageRec;
        if (verboseOutput) {
          Serial.print("SUCCESS: PD Voltage set to: ");
          Serial.println(setVoltage);
        }
        writeSettings(); //Save to EEPROM
      }
      else {
        Serial.println("ERROR: Voltage must be 5, 9, 12, 15 or 20");
      }
    }

    else if (cmd.startsWith("current=")) {
      int currentRec = cmd.substring(8).toInt();
      if (currentRec >= 0 and currentRec <= 100){
        current = currentRec;
        if (verboseOutput) {
          Serial.print("SUCCESS: Current set to: ");
          Serial.print(current);
          Serial.println("%");
        }
        writeSettings(); //Save to EEPROM
      }
      else {
        Serial.println("ERROR: Current needs to be a value from 0-100");
      }
    }

    else if (cmd.startsWith("speed=")) {  //max speed for position control
      float speedRec = cmd.substring(6).toFloat();
      if (speedRec >= 0 and speedRec <= 5000){
        positionSpeed = speedRec;
        if (verboseOutput) {
          Serial.print("SUCCESS: Position speed value set to: ");
          Serial.println(positionSpeed);
        }
        writeSettings(); //Save to EEPROM
      }
      else {
        Serial.println("ERROR: Position speed must be between 0 and 5000 deg/s");
      }
    }

    else if (cmd.startsWith("standstill_mode=")) {
      String standstillModeRec = cmd.substring(16);
      if (standstillModeRec == "NORMAL" or standstillModeRec == "FREEWHEELING" or standstillModeRec == "BRAKING" or standstillModeRec == "STRONG_BRAKING"){
        standstillMode = standstillModeRec;
        if (verboseOutput) {
          Serial.print("SUCCESS: Standstill mode set to: ");
          Serial.println(standstillMode);
        }
        writeSettings(); //Save to EEPROM
      }
      else {
        Serial.println("ERROR: Standstill mode must be NORMAL, FREEWHEELING, BRAKING or STRONG_BRAKING");
      }
    }

    else if (cmd.startsWith("closed_loop_type=")) {
      String closedLoopTypeRec = cmd.substring(17);
      if (closedLoopTypeRec == "OPEN_LOOP" or closedLoopTypeRec == "MOVE_FROM_ENC" or closedLoopTypeRec == "CLOSED_LOOP"){
        closedLoopType = closedLoopTypeRec;
        if (verboseOutput) {
          Serial.print("SUCCESS: Closed loop mode set to: ");
          Serial.println(closedLoopType);
        }
        writeSettings(); //Save to EEPROM
      }
      else {
        Serial.println("ERROR: MOVE_FROM_ENC must be OPEN_LOOP, MOVE_FROM_ENC Or CLOSED_LOOP");
      }
    }

    else if (cmd.startsWith("mappingDirection=")) {
      int mappingDirectionRec = cmd.substring(17).toInt();
      if (mappingDirectionRec == 0 or mappingDirectionRec == 1){
        mappingDirection = mappingDirectionRec;
        if (verboseOutput) {
          Serial.print("SUCCESS: Mapping direction mode set to: ");
          Serial.println(mappingDirection);
        }
        writeSettings(); //Save to EEPROM
      }
      else {
        Serial.println("ERROR: Mapping direction must be 0 or 1");
      }
    }

    else if (cmd.startsWith("verboseOutput=")) {
      String verboseOutputRec = cmd.substring(14);
      if (verboseOutputRec == "0" or verboseOutputRec == "FALSE" or verboseOutputRec == "disabled"){
        verboseOutput = 0;
        //Serial.println("SUCCESS: verbose output disabled");
        writeSettings(); //Save to EEPROM
      } else if (verboseOutputRec == "1" or verboseOutputRec == "TRUE" or verboseOutputRec == "enabled"){
        verboseOutput = 1;
        Serial.println("SUCCESS: verbose output enabled");
        writeSettings(); //Save to EEPROM
      }
      else {
        Serial.println("ERROR: verbose output must be set to 0, 1, FALSE or TRUE");
      }
    }

    else if (cmd.startsWith("HELP") or cmd.startsWith("help") or cmd.startsWith("Help")) {
      Serial.println("");
      Serial.println("List of available commands:");
      Serial.println("deg= xxx.x          : move to position, in degrees");
      Serial.println("vel= xxxx           : spin at set velocity");
      Serial.println("steps_per_rev= xxx  : Steps per revolution of your motor, 200 or 400");
      Serial.println("enable= x           : enable/disable driver, 0 or 1");
      Serial.println("microsteps= xxx     : set microsteps per step, 1, 4, 8, 16, 32, 64. 1268 or 256");
      Serial.println("voltage= xx         : set USB PD Voltage, 5, 9, 12, 15 or 20");
      Serial.println("current= xxx        : set driver current as a percentage, 0-100");
      Serial.println("speed= xxxx         : set position control speed in degrees per second, 0-5000");
      Serial.println("standstill_mode= x  : set standstill mode of driver, NORMAL, FREEWHEELING, BRAKING or STRONG_BRAKING");
      Serial.println("closed_loop_type=x  : control type, OPEN_LOOP, MOVE_FROM_ENC or CLOSED_LOOP (closed loop not done yet)");
      Serial.println("mappingDirection=x  : reverses the motor direction (to sync with encoder), 0 or 1");
      Serial.println("verboseOutput=x     : enables or disabled confirmation messages, 0 or 1");
      Serial.println("values              : output current setup");
      Serial.println("led_flash           : flash LED1 twice");
      Serial.println("get_angle           : return current total encoder angle in degrees");
      Serial.println("");
    }

    else if (cmd.startsWith("VALUES") or cmd.startsWith("values") or cmd.startsWith("Values")) {
      Serial.println("");
      Serial.println("Current Driver Setup:");
      Serial.print("steps_per_rev = ");
      Serial.println(stepsPerRev);
      Serial.print("enable = ");
      Serial.println(enabled1);
      Serial.print("microsteps = ");
      Serial.println(microsteps);
      Serial.print("voltage = ");
      Serial.println(setVoltage);
      Serial.print("current = ");
      Serial.println(current);
      Serial.print("speed = ");
      Serial.println(positionSpeed);
      Serial.print("standstill_mode = ");
      Serial.println(standstillMode);
      Serial.print("closed_loop_type = ");
      Serial.println(closedLoopType);
      Serial.print("mappingDirection = ");
      Serial.println(mappingDirection);
      Serial.print("verboseOutput = ");
      Serial.println(verboseOutput);
      
      Serial.println("");
    }

    else if (cmd.startsWith("led_flash")) {
      digitalWrite(LED1, HIGH);
      delay(300);
      digitalWrite(LED1, LOW);
      delay(200);
      digitalWrite(LED1, HIGH);
      delay(300);
      digitalWrite(LED1, LOW);
    }

    else if (cmd.startsWith("get_angle")) {
      Serial.println(angle);
    }

    else {
      Serial.println("ERROR: Unknown command, enter 'help' for a list of commands");
    }
  }
}

// Returns step delay in microseconds
unsigned long getStepDelay(float deg_per_sec, int steps_per_rev, int microsteps) {
  // Total microsteps per revolution
  double total_steps_per_rev = steps_per_rev * microsteps;

  // Steps per second needed
  double steps_per_sec = (deg_per_sec / 360.0) * total_steps_per_rev;

  // Prevent divide by zero
  if (steps_per_sec <= 0) return 0;

  // Delay between steps in microseconds
  unsigned long delay_us = (1.0 / steps_per_sec) * 1e6;

  return delay_us;
}


void readEncoder(){
  int raw_counts;
  static int prev_raw_counts = 0;
  static signed long revolutions = 0;
//  float angle;
    // Request the raw encoder counts from the AS5600 sensor
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(0x0C); // Register address for raw angle output (0x0C)
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDRESS, 2); // Request 2 bytes of data
  if (Wire.available() >= 2) {
    raw_counts = Wire.read() << 8 | Wire.read(); // Combine two bytes to get the counts value
  }

  // Check to see if it has gone past "home" over one full rotation
  if (prev_raw_counts > 3000 && raw_counts < 1000) {
    revolutions++;
  } else if (prev_raw_counts < 1000 && raw_counts > 3000) {
    revolutions--;
  }

  // Update the previous raw counts
  prev_raw_counts = raw_counts;
  total_encoder_counts = raw_counts + (4096 * revolutions);

  //convert to angle (and apply initial offset)
  angle = ((total_encoder_counts-encoder_offset) * 360.0) / 4096.0;
}



//takes saved values impliments them (e.g after settings changed)
void configureSettings(){

  if (setVoltage == 5){
                                 //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, HIGH);  //  1    0     0     0     0
  } else if (setVoltage == 9){
                                //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, LOW);  //  1    0     0     0     0
      digitalWrite(CFG2, LOW);  //  -    0     0     1     1
      digitalWrite(CFG3, LOW);  //  -    0     1     1     0
  } else if (setVoltage == 12){
                                //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, LOW);  //  1    0     0     0     0
      digitalWrite(CFG2, LOW);  //  -    0     0     1     1
      digitalWrite(CFG3, HIGH); //  -    0     1     1     0
  } else if (setVoltage == 15){
                                //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, LOW);  //  1    0     0     0     0
      digitalWrite(CFG2, HIGH); //  -    0     0     1     1
      digitalWrite(CFG3, HIGH); //  -    0     1     1     0
  } else if (setVoltage == 20){
                                //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, LOW);  //  1    0     0     0     0
      digitalWrite(CFG2, HIGH); //  -    0     0     1     1
      digitalWrite(CFG3, LOW);  //  -    0     1     1     0
  }

  stepper_driver.setRunCurrent(current);
  stepper_driver.setMicrostepsPerStep(microsteps);
  stepper_driver.setStallGuardThreshold(stallThreshold);

  if (standstillMode == "NORMAL"){ stepper_driver.setStandstillMode(stepper_driver.NORMAL);}
  else if (standstillMode == "FREEWHEELING"){ stepper_driver.setStandstillMode(stepper_driver.FREEWHEELING);}
  else if (standstillMode == "BRAKING"){ stepper_driver.setStandstillMode(stepper_driver.BRAKING);}
  else if (standstillMode == "STRONG_BRAKING"){ stepper_driver.setStandstillMode(stepper_driver.STRONG_BRAKING);}

}


//Read saved settings from EEPROM//
void readSettings(){ 

  preferences.begin("settings", false); //open the settings namespace

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



//save settings to flash
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
  
  configureSettings(); //use new settings
}
