/*
 * USB PD Stepper Slider demo code
 * TODO:
 *  - Stall gaurd for sensorless homing
 *  - Disable interupt when reading from set_speed (better handling in general could set speed in handler?)
*/



#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include <TMC2209.h> //  https://github.com/janelia-arduino/TMC2209/tree/main

const char *ssid = "Slider";
const char *password = "password";

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
#define DIAG    20
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

//flashing LED
long lastFlash = 0;
int flashInt = 100;
bool flashState = 0;

//button read and debounce
bool incButtonState = HIGH;
bool decButtonState = HIGH;
bool resetButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

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
  

  //Setup serial comms with TMC2209
  pinMode(MS1, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(TMC_EN, OUTPUT);  //maybe not needed as this done over serial
  digitalWrite(TMC_EN, LOW); //Enabled here and later enabled/disabled over UART

  digitalWrite(MS1, LOW); //used to set serial address in UART mode
  digitalWrite(MS2, LOW);
  
  stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE, TMC2209::SERIAL_ADDRESS_0, TMC_RX, TMC_TX);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.enableAutomaticCurrentScaling(); /////////TESTING
//  stepper_driver.enableCoolStep();
  stepper_driver.disable();

  ///////////////////////////////////////
  // Can't auto enter bootloader if
  // serial.begin has been called
  // so hold down SW2 on boot to enable serial 
  // if you need to read outputs but not program
  ///////////////////////////////////////
  //(Can also manually enter bootloader mode by holding BOOT, press RST, release BOOT)

  if (digitalRead(SW2) == LOW){  //push = LOW
    Serial.begin(115200);
    Serial.println("Code Starting");
  }
  
  // Set up ESP32 as an Access Point
  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip);

  // Route to handle slider position update
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("slider", true)) {
      AsyncWebParameter* sliderParam = request->getParam("slider", true);
      String sliderValue = sliderParam->value();
      set_speed = sliderValue.toInt(); //convert to int
      Serial.println("Slider value: " + sliderValue);
    }
    request->send(200); // Respond with HTTP 200 OK
  });

  // Serve HTML page with JavaScript for slider position update
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.begin();


   
}

void loop() {
  //flash LED to show code is running
  if (millis() - lastFlash > flashInt){
    lastFlash = millis();
    digitalWrite(LED1, flashState);
    if (flashState == 0){
      flashState = 1;
    } else {
      flashState = 0;
    }
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    bool currentIncButtonState = digitalRead(SW3);
    bool currentDecButtonState = digitalRead(SW1);
    bool currentResetButtonState = digitalRead(SW2);
  
    if (currentIncButtonState != incButtonState) {
      incButtonState = currentIncButtonState;
      if (incButtonState == LOW) {
        set_speed = set_speed + 10;
        if (set_speed > 100){
          set_speed = 100;
        }
        Serial.print("Set speed increased to: ");
        Serial.println(set_speed);
      }
    }
  
    if (currentDecButtonState != decButtonState) {
      decButtonState = currentDecButtonState;
      if (decButtonState == LOW) {
        set_speed = set_speed -10;
        if (set_speed < -100){
          set_speed = -100;
        }
        Serial.print("Set speed decreased to: ");
        Serial.println(set_speed);
      }
    }
  
    if (currentResetButtonState != resetButtonState) {
      resetButtonState = currentResetButtonState;
      if (resetButtonState == LOW) {
        set_speed = 0;
        Serial.println("Counter reset");
      }
    }
  }
  
  
  PGState = digitalRead(PG);

  //should probably only enable/disable on state change
  if (PGState == 0){  //only enable motor if correct voltage seen (to stop motor running off laptop 5V when programming etc)
    stepper_driver.enable();
    int temp_speed = set_speed;
    Serial.println(temp_speed);
    if (temp_speed == 0){
      stepper_driver.moveAtVelocity(0);
    } else if (set_speed > 0){
      stepper_driver.enableInverseMotorDirection();
      stepper_driver.moveAtVelocity(temp_speed*600);
    } else{
      stepper_driver.disableInverseMotorDirection();
      stepper_driver.moveAtVelocity(temp_speed*-600);
    }
  }
  else {
    stepper_driver.disable();
  }

  delay(30);
  
}
