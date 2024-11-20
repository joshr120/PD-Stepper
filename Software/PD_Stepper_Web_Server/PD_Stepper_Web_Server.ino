/*
 * PD Stepper Webpage Example:
 * 
 *********  Software Version 1.0 ******************
 * 
 *    How to Use:
 * 1. Connect to created WiFi Network named "PD Stepper"
 * 2. On a browser visit 192.168.4.1
 * 
 *  For more info and to purchase PD Stepper kits visit:
 *  https://thingsbyjosh.com
 * 
 * TODO:
 * - More TMC error conditions (including power bad?)
 * - Stall guard disabled at lower speeds
 * - Fix non smooth position control (due to webserver interuptions)
*/



#include <WiFi.h>
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include <AsyncTCP.h>  //https://github.com/me-no-dev/AsyncTCP

//#include <ESPmDNS.h>

#include <TMC2209.h> //  https://github.com/janelia-arduino/TMC2209/tree/main

#include <Preferences.h> //for saving to flash (instead of old EEPROM lib)

Preferences preferences;


//access point SSID and password (password = "" for no password)
const char *ssid = "PD Stepper";
const char *password = "";

#include "index_html.h"

AsyncWebServer server(80);

//TMC2209 setup
TMC2209 stepper_driver;
HardwareSerial & serial_stream = Serial2;
const long SERIAL_BAUD_RATE = 115200;

const uint8_t RUN_CURRENT_PERCENT = 100; //how much current to run at (0-100%)

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
unsigned long lastEncRead = 0;

int mainFreq = 10; //Scheduled frequency = 100hz (for slower tasks, encoder reading etc)

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

//Values received from websever save command
String enabled1 = "enabled";
String setVoltage = "12";
String microsteps = "32";
String current = "30";
String stallThreshold = "10";
String standstillMode = "NORMAL";

//Varaiables for position control (open loop)
signed long setPoint = 0;
signed long CurrentPosition = 0;
unsigned long lastStep = 0;

//read state of PG pin to display on webpage
String readPGState(){
  PGState = digitalRead(PG);
  if (PGState == 0){
    return ("Power Good");
  } else {
    return ("Power Bad");
  }
}

//read VBUS voltage to display on webpage
String readVoltage(){
  int ADCValue = analogRead(VBUS);
  VBusVoltage = ADCValue * (VREF / 4096.0) / DIV_RATIO;
  return String(VBusVoltage)+"V";
}

//read encoder pos to display on webpage
String readEncoderPos(){
  readEncoder();
  return String(total_encoder_counts);
}

String readTMCStatus(){
  bool hardware_disabled = stepper_driver.hardwareDisabled();
  if (hardware_disabled){
   return ("Hardware Disabled");
  }

  TMC2209::Status status = stepper_driver.getStatus();
  if (status.over_temperature_warning){
    return ("Over Temp Warning");
  }
  else if (status.over_temperature_shutdown){
    return ("Over Temp Shutdown");
  }
  return ("No Errors");
}

String readStallStatus(){
  return String(stepper_driver.getStallGuardResult());
//  if (digitalRead(DIAG) == HIGH){
//    return ("Stalled");
//  } else{
//    return ("Not Stalled");
//  }
}


//updates placeholder varibles in the HTML code //puts current arduino settings into webpage
//Puts stored values into the HTML value
String processor(const String& var)
{
  if(var == "enabled1"){
    if (enabled1 == "enabled"){
      return "checked";
    } else { return "";}
  }

  if (var == "microsteps"){
    return String(microsteps);
  }

  if(var == "voltage"){
     return String(setVoltage);
  }

  if(var == "current"){
     return String(current);
  }

  if(var == "stall_threshold"){
     return String(stallThreshold);
  }

  if(var == "standstill_mode"){
     return String(standstillMode);
  }
}


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

  delay(200); //delay needed before "Serial.begin" to ensure bootloader mode entered correctly. Otherwise bootloader mode may need to be manually entered by holding BOOT, press RST, release BOOT
  Serial.begin(115200);
  Serial.println("Code Starting");
  
  // Set up ESP32 as an Access Point
  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip);

  
  // Serve HTML page with JavaScript for updating values
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/powergood", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readPGState().c_str());
  });
  server.on("/voltage", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readVoltage().c_str());
  });
  server.on("/position", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readEncoderPos().c_str());
  });
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTMCStatus().c_str());
  });
  server.on("/stallguard", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readStallStatus().c_str());
  });

  // Route to handle slider position update
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("slider", true)) {
      AsyncWebParameter* sliderParam = request->getParam("slider", true);
      String sliderValue = sliderParam->value();
      set_speed = sliderValue.toInt(); //convert to int
      Serial.println("Slider value: " + sliderValue);
      if (digitalRead(PG)==0){ //only move if PG is good
        stepper_driver.moveAtVelocity(set_speed*(microsteps.toInt()));
      }
      stepper_driver.moveAtVelocity(set_speed*(microsteps.toInt()));
    }

    if (request->hasParam("positionControl", true)) {  //should not move directly here, raise a flag or setpoint instead!!!
      AsyncWebParameter* sliderParam = request->getParam("positionControl", true);
      String posValue = sliderParam->value();
      stepper_driver.moveAtVelocity(0); //stop moving incase still moving from velocity control mode
      
      if (posValue == "1"){
        setPoint -= 25600; //must be factor of 256 due to how the microsteping calcs work
      } else if (posValue == "2"){
        setPoint -= 12800;
      } else if (posValue == "3"){
        setPoint += 12800;
      } else if (posValue == "4"){
        setPoint += 25600;
      } 
    }
    request->send(200); // Respond with HTTP 200 OK
  });

  //handle post request of saving form
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam("enabled1", true)) { //when checked "enabled1" variable is present
      enabled1 = "enabled";
    } else {
      enabled1 = "disabled";
    }
    if (request->hasParam("setvoltage", true)) {
      inputMessage = request->getParam("setvoltage", true)->value();
      setVoltage = inputMessage;     
      stepper_driver.moveAtVelocity(0); //stop when voltage changed
    }
    if (request->hasParam("microsteps", true)) {
      inputMessage = request->getParam("microsteps", true)->value();
      microsteps = inputMessage;

      stepper_driver.moveAtVelocity(0); //stop when microsteps changed
    }
    if (request->hasParam("current", true)) {
      inputMessage = request->getParam("current", true)->value();
      current = inputMessage;
    }
    if (request->hasParam("stall_threshold", true)) {
      inputMessage = request->getParam("stall_threshold", true)->value();
      stallThreshold = inputMessage;
    }
    if (request->hasParam("standstill_mode", true)) {
      inputMessage = request->getParam("standstill_mode", true)->value();
      standstillMode = inputMessage;

      writeSettings();  //only save to EEPROM if values from "save" received
    }
//    request->send(200, "text/html", "<h2>Saved</h2><br><a href=\"/\">Return to Home Page</a>"); //respond with save page
    request->redirect("/"); // Redirct back to main page to avaid having /save in URL
  });

  server.begin();

  digitalWrite(LED1, HIGH); //flash LED after setep complete
  delay(200);
  digitalWrite(LED1, LOW); 

}

void loop() {

  if (millis() - lastEncRead >= mainFreq){ //main loop
    lastEncRead = millis();
    //readEncoder(); //need to constantly read encoder in order to catch wrap around

    digitalWrite(LED2, digitalRead(DIAG)); //Stall detection

    PGState = digitalRead(PG);
    if (PGState == LOW and enabled1 == "enabled" and enabledState == 0){ //only enable if PD chip reports good
      stepper_driver.enable();
      enabledState = 1;
    } else if ((PGState == HIGH or enabled1 == "disabled") and enabledState == 1){ //Disable stepper if box unchecked or PG is high
      stepper_driver.disable();
      enabledState = 0;
    }
  }

  

  //position control done here (open-loop for now)
  int delaySpeed = 4500; //delay speed smaller value = faster
  int microSteps = microsteps.toInt();
  int delaySpeedAdjusted = delaySpeed/microSteps; //adjusting speed depending on microsteps
  
  if (setPoint > CurrentPosition){ //position control
    if (micros()-lastStep > delaySpeedAdjusted){ //always move same speed regardless of what microsteps set
      digitalWrite(DIR, LOW);
      digitalWrite(STEP, state);
      state = !state;
      CurrentPosition = CurrentPosition + (256/microSteps);  //update current position taking into account microsteps set
      lastStep = micros();
    }
  } else if (setPoint < CurrentPosition){ //position control
    if (micros()-lastStep > delaySpeedAdjusted){
      digitalWrite(DIR, HIGH);
      digitalWrite(STEP, state);
      state = !state;
      CurrentPosition = CurrentPosition - (256/microSteps);  //update current position taking into account microsteps set
      lastStep = micros();
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
        buttonSpeed = buttonSpeed + 30;
        if (buttonSpeed > 330){
          buttonSpeed = 330;
        }
        stepper_driver.moveAtVelocity(buttonSpeed*(microsteps.toInt()));
      }
    }
  
    if (currentDecButtonState != decButtonState) {
      decButtonState = currentDecButtonState;
      if (decButtonState == LOW) {
        buttonSpeed = buttonSpeed -30;
        if (buttonSpeed < -330){
          buttonSpeed = -330;
        }
        stepper_driver.moveAtVelocity(buttonSpeed*(microsteps.toInt()));
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

  // Convert raw counts to angle in degrees if that is what's needed
//  angle = (raw_counts * 360.0) / 4096.0;

  // Check to see if it has gone past "home" over one full rotation
  if (prev_raw_counts > 3000 && raw_counts < 1000) {
    revolutions++;
  } else if (prev_raw_counts < 1000 && raw_counts > 3000) {
    revolutions--;
  }

  // Update the previous raw counts
  prev_raw_counts = raw_counts;
  total_encoder_counts = raw_counts + (4096 * revolutions);
}



//takes saved values impliments them (e.g after settings changed)
void configureSettings(){

  if (setVoltage == "5"){
                                 //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, HIGH);  //  1    0     0     0     0
  } else if (setVoltage == "9"){
                                //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, LOW);  //  1    0     0     0     0
      digitalWrite(CFG2, LOW);  //  -    0     0     1     1
      digitalWrite(CFG3, LOW);  //  -    0     1     1     0
  } else if (setVoltage == "12"){
                                //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, LOW);  //  1    0     0     0     0
      digitalWrite(CFG2, LOW);  //  -    0     0     1     1
      digitalWrite(CFG3, HIGH); //  -    0     1     1     0
  } else if (setVoltage == "15"){
                                //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, LOW);  //  1    0     0     0     0
      digitalWrite(CFG2, HIGH); //  -    0     0     1     1
      digitalWrite(CFG3, HIGH); //  -    0     1     1     0
  } else if (setVoltage == "20"){
                                //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
      digitalWrite(CFG1, LOW);  //  1    0     0     0     0
      digitalWrite(CFG2, HIGH); //  -    0     0     1     1
      digitalWrite(CFG3, LOW);  //  -    0     1     1     0
  }

  stepper_driver.setRunCurrent(current.toInt());
  stepper_driver.setMicrostepsPerStep(microsteps.toInt());
  stepper_driver.setStallGuardThreshold(stallThreshold.toInt());

  if (standstillMode == "NORMAL"){ stepper_driver.setStandstillMode(stepper_driver.NORMAL);}
  else if (standstillMode == "FREEWHEELING"){ stepper_driver.setStandstillMode(stepper_driver.FREEWHEELING);}
  else if (standstillMode == "BRAKING"){ stepper_driver.setStandstillMode(stepper_driver.BRAKING);}
  else if (standstillMode == "STRONG_BRAKING"){ stepper_driver.setStandstillMode(stepper_driver.STRONG_BRAKING);}
}

//Read saved settings from EEPROM//
void readSettings(){ 

  preferences.begin("settings", false); //open the settings namespace

  enabled1 = preferences.getString("enable", ""); 
  
  if (enabled1 == ""){ //EEPROM has not been saved to before so save defaults
    preferences.end(); //close to write EEPROM can open again
    enabled1 = "enabled";
    setVoltage = "12";
    microsteps = "32";
    current = "30";
    stallThreshold = "10";
    standstillMode = "NORMAL";
    writeSettings();
  } else {
    Serial.println("Settings found in EEPROM");
    setVoltage = preferences.getString("voltage", ""); 
    microsteps = preferences.getString("microsteps", ""); 
    current = preferences.getString("current", ""); 
    stallThreshold = preferences.getString("stallThreshold", ""); 
    standstillMode = preferences.getString("standstillMode", ""); 
    preferences.end();
  }
}



//save settings to flash
void writeSettings(){ //easiest (not best) is to write strings and then re-calculate values which use that

  preferences.begin("settings", false);

  preferences.putString("enable", enabled1); 
  preferences.putString("voltage", setVoltage); 
  preferences.putString("microsteps", microsteps); 
  preferences.putString("current", current); 
  preferences.putString("stallThreshold", stallThreshold); 
  preferences.putString("standstillMode", standstillMode); 

  Serial.println("Saving settings to flash");

  preferences.end();
  
  configureSettings(); //use new settings
}
