
/*  Code to test the basic functionality of the USB PD Stepper Driver and Controller
 * 
 * Note this only uses the step and dir pins to interface with the TMC2209. See other examples for more advanced serial commms
 * by Things by Josh 2024
 */

 
#include <Wire.h> //For I2C for encoder

/////////////////////////
//     Pin Defines     //
/////////////////////////

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

//AS5600 Hall Effect Encoder
#define SCL     9
#define SDA     8

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
int stepDelay = 100;  //delay (microseconds) between steps (AKA Speed)
int motorDir = 0; //Stepper motor direction

//PD Trigger (CH224K)
bool PGState = 0;

//AS5600 Hall Effect Encoder
#define AS5600_ADDRESS 0x36 // I2C address of the AS5600 sensor
signed long total_encoder_counts = 0;

//Other
float VBusVoltage = 0;
float VREF = 3.3;
const float DIV_RATIO = 0.1189427313;  //20k&2.7K Voltage Divider

//LED Flash
long lastFlash = 0;
int flashInt = 100;
bool flashState = 0;

//Step state
bool lastState = 0;

void setup() {

  //TMC2209 Stepper Driver Setup (Simple not using serial comms)
  pinMode(TMC_EN, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(SPREAD, OUTPUT);
  pinMode(INDEX, INPUT);
  pinMode(DIAG, INPUT); 

  digitalWrite(TMC_EN, HIGH); //High to disable on startup
  digitalWrite(MS1, LOW); //Microstep resolution configuration (internal pull-down resistors: MS2, MS1: 00: 1/8, 01: 1/32, 10: 1/64 11: 1/16
  digitalWrite(MS2, LOW);
  digitalWrite(DIR, motorDir); //set direction at start

  
  //PD Trigger Setup
  pinMode(PG, INPUT);
  pinMode(CFG1, OUTPUT);
  pinMode(CFG2, OUTPUT);
  pinMode(CFG3, OUTPUT);
                            //  5V   9V   12V   15V   20V  (Can also be changed on the fly)
  digitalWrite(CFG1, LOW);  //  1    0     0     0     0
  digitalWrite(CFG2, LOW);  //  -    0     0     1     1
  digitalWrite(CFG3, HIGH); //  -    0     1     1     0


  //General/Other Setup
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
//  pinMode(AUX1, INPUT);  //can be set to anything
//  pinMode(AUX2, INPUT);  //can be set to anything
  pinMode(NTC, INPUT);
  pinMode(VBUS, INPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  delay(500); //delay needed before "Serial.begin" to ensure bootloader mode entered correctly. Otherwise bootloader mode may need to be manually entered by holding BOOT, press RST, release BOOT
  Serial.begin(115200);
  Serial.println("Code Starting");

    //AS5600 Hall Encoder Setup
  Wire.begin(SDA, SCL);  //start wire with earlier defined pins


}

void loop() {
  readVoltage();  //read and output voltage as well as PG status from PD trigger
  runMotor();     //run the stepper motor at set speed and direction (ensure readVoltage() called first to get PGState)
  readEncoder();

  if (millis() - lastFlash > flashInt){
    lastFlash = millis();
    digitalWrite(LED1, flashState);
    if (flashState == 0){
      flashState = 1;
    } else {
      flashState = 0;
    }
  }

  delay(100);
  
}

//run the stepper motor at set speed and direction (ensure readVoltage() called first to get PGState)
void runMotor(){
  PGState = digitalRead(PG); //Low = power good!
  if (PGState == 0){  //only enable motor if correct voltage seen (to stop motor running off laptop 5V when programming)
    digitalWrite(TMC_EN, LOW); //low to enable (should really only do this on state change)
    digitalWrite(DIR, digitalRead(SW3)); //low to enable (should really only do this on state change)
    if (micros() - lastStep > stepDelay){
      lastStep = micros();
      digitalWrite(STEP, lastState);
      if (lastState == 0){
        lastState = 1;
      } else {
        lastState = 0;
      }
    }
  } else{
    digitalWrite(TMC_EN, HIGH); //disable stepper as voltage not as expected (should really only do this on state change)
  }
}

//read and output voltage as well as PG status from PD trigger
void readVoltage(){
  int ADCValue = analogRead(VBUS);
  VBusVoltage = ADCValue * (VREF / 4096.0) / DIV_RATIO;
  PGState = digitalRead(PG); //Low = power good!


  Serial.print("Voltage Read: ");
  Serial.print(VBusVoltage);
  Serial.println("V");

  Serial.print("PG Status: ");
  Serial.println(PGState);
}

//read the AS5600 encoder via I2C
void readEncoder(){
  int raw_counts;
  static int prev_raw_counts = 0;
  static signed long revolutions = 0;
  float angle;
    // Request the raw encoder counts from the AS5600 sensor
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(0x0C); // Register address for raw angle output (0x0C)
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDRESS, 2); // Request 2 bytes of data
  if (Wire.available() >= 2) {
    raw_counts = Wire.read() << 8 | Wire.read(); // Combine two bytes to get the counts value
  }

  // Convert raw counts to angle in degrees if that is what's needed
  angle = (raw_counts * 360.0) / 4096.0;

  // Check to see if it has gone past "home" over one full rotation
  if (prev_raw_counts > 3000 && raw_counts < 1000) {
    revolutions++;
  } else if (prev_raw_counts < 1000 && raw_counts > 3000) {
    revolutions--;
  }

  // Update the previous raw counts
  prev_raw_counts = raw_counts;

  total_encoder_counts = raw_counts + (4096 * revolutions);\

  Serial.print("Encoder Counts: ");
  Serial.println(total_encoder_counts);
  Serial.println("");  
}
