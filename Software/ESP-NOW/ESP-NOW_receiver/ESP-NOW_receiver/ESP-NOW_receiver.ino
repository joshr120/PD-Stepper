/*
  ESP-NOW code from: https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
*/

#include <esp_now.h>
#include <WiFi.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    signed long pos;
} struct_message;

// Create a struct_message called myData
struct_message myData;

/////////////////////////
//     Pin Defines     //
/////////////////////////

//TMC2209 Stepper Driver
#define TMC_EN  21 //Low to enable motor, high to disable)
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

////////////////////////////
// Global Variable Defines //
////////////////////////////
//TMC2209 Stepper Driver
unsigned long lastStep = 0; //time of last step
int motorDir = 0; //Stepper motor direction
bool lastState = 0;

//USB PD Trigger
bool PGState = 0; //power good singal from trigger IC

//Other
signed long setPos = 0; //pos received from sender
int minDelay = 30; //max speed motor will turn at to reach setpoint
signed long currentPos = 0; //current position (not using encoder yet)

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
//  Serial.print("Position received: ");
//  Serial.println(myData.pos);
  setPos = myData.pos;
}
 
void setup() {
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
  //PD Trigger Setup
  pinMode(PG, INPUT);
  pinMode(CFG1, OUTPUT);
  pinMode(CFG2, OUTPUT);
  pinMode(CFG3, OUTPUT);
                            //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
  digitalWrite(CFG1, LOW);  //  1    0     0     0     0
  digitalWrite(CFG2, LOW);  //  -    0     0     1     1
  digitalWrite(CFG3, HIGH); //  -    0     1     1     0

  //Stepper simple setup (no serial comms)
  pinMode(TMC_EN, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS1, OUTPUT);

  digitalWrite(TMC_EN, HIGH); //High to disable on startup
  digitalWrite(MS1, HIGH); //Microstep resolution configuration (internal pull-down resistors: MS2, MS1: 00: 1/8, 01: 1/32, 10: 1/64 11: 1/16
  digitalWrite(MS2, LOW);
  
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);


   ///////////////////////////////////////
  // Can't auto enter bootloader if
  // serial.begin has been called
  // so hold down SW2 on boot to enable serial 
  // (wont auto enter bootloader mode)
  ///////////////////////////////////////
  //(Can also manually enter bootloader mode by holding BOOT, press RST, release BOOT)

  if (digitalRead(SW2) == LOW){  //push = LOW
    Serial.begin(115200);
  }
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  noInterrupts();
  signed long setPoint = setPos*3.125;
  interrupts();
  signed long Error = currentPos - setPoint;
  int minDelay = 30;

  if (abs(Error) < (550-minDelay)){ //200 is essentially the "Kp" gain
    minDelay = 550 - abs(Error);
  }
  
  PGState = digitalRead(PG);
  if (currentPos != setPoint and PGState == 0){ //move motor
//    if (micros() - lastStep > minDelay){  //min time between steps (or use delay instead)
      digitalWrite(TMC_EN, LOW); //low to enable
      if (currentPos < setPoint){
        digitalWrite(DIR, HIGH);
        currentPos++;
      } else {
        digitalWrite(DIR, LOW);
        currentPos--;
      }
      lastStep = micros();
      digitalWrite(STEP, lastState);
      if (lastState == 0){
        lastState = 1;
      } else {
        lastState = 0;
      }
//    }
            
  } else{
    digitalWrite(TMC_EN, HIGH); //disable motor when setpoint reached
  }

  delayMicroseconds(minDelay); //set min time between steps
}
