

/*  Simple code to run a stepper motor with buttons using the USB PD Stepper Driver and Controller
 * 
 * by Things by Josh 2024
 */



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


////////////////////////////
// Global Variable Defines //
////////////////////////////
//TMC2209 Stepper Driver
unsigned long lastStep = 0; //time of last step
int motorDir = 0; //Stepper motor direction
bool lastState = 0;

//USB PD Trigger
bool PGState = 0; //power good singal from trigger IC

//buttons and motor control
int counter = 0;
bool incButtonState = HIGH;
bool decButtonState = HIGH;
bool resetButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

int delays[] = {0, 2000, 1000, 700, 400, 200, 100, 75, 50, 40, 30}; //list of speeds (higher = slower, 0 = stop)
int numSpeeds = 10;

long lastFlash = 0;
int flashInt = 100;
bool flashState = 0;




void setup() {
  // put your setup code here, to run once:
  
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


  delay(500); //delay needed before "Serial.begin" to ensure bootloader mode entered correctly. Otherwise bootloader mode may need to be manually entered by holding BOOT, press RST, release BOOT
  Serial.begin(115200);
  Serial.println("Code Starting");

  
  
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
  
  // Read and doubounce inputs and incriment counter
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    bool currentIncButtonState = digitalRead(SW3);
    bool currentDecButtonState = digitalRead(SW1);
    bool currentResetButtonState = digitalRead(SW2);

    if (currentIncButtonState != incButtonState) {
      incButtonState = currentIncButtonState;
      if (incButtonState == LOW) {
        counter++;
        if (counter > numSpeeds){
          counter = numSpeeds;
        }
        Serial.print("Counter increased to: ");
        Serial.println(counter);
      }
    }

    if (currentDecButtonState != decButtonState) {
      decButtonState = currentDecButtonState;
      if (decButtonState == LOW) {
        counter--;
        if (counter < (numSpeeds*-1)){
          counter = (numSpeeds*-1);
        }
        Serial.print("Counter decreased to: ");
        Serial.println(counter);
      }
    }

    if (currentResetButtonState != resetButtonState) {
      resetButtonState = currentResetButtonState;
      if (resetButtonState == LOW) {
        counter = 0;
        Serial.println("Counter reset");
      }
    }
  }


    

    //read PG to check if motor shoulf be enabled (this should be scheduled to not run as often)
    PGState = digitalRead(PG);
    if (counter != 0 and PGState == 0){  //only enable motor if correct voltage seen (to stop motor running off laptop 5V when programming)
      digitalWrite(TMC_EN, LOW); //low to enable (should really only do this on state change)
      
      int abs_counter = 0;
      if (counter > 0){ //set direction (should only do this on Dir change really)
        digitalWrite(DIR, HIGH);
        abs_counter = counter;
      } else {
        digitalWrite(DIR, LOW);
        abs_counter = counter*-1;
      }
      
      if (micros() - lastStep > delays[abs_counter]){
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
