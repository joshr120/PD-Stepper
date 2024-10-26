/*
  ESP-NOW code from: https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
*/

#include <esp_now.h>
#include <WiFi.h>

#include <Wire.h> //For I2C for encoder

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x63, 0xE8, 0x33, 0x60, 0x74, 0x5C}; //REPLACE WITH YOUR SENDER 

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  signed long pos;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

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
//USB PD Trigger
bool PGState = 0; //power good singal from trigger IC

long lastFlash = 0;
int flashInt = 100;
bool flashState = 0;

//position to send
signed long pos = 0;

//AS5600 Hall Effect Encoder
#define AS5600_ADDRESS 0x36 // I2C address of the AS5600 sensor
signed long total_encoder_counts = 0;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//  Serial.print("\r\nLast Packet Send Status:\t");
//  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
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

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);


  delay(500); //delay needed before "Serial.begin" to ensure bootloader mode entered correctly. Otherwise bootloader mode may need to be manually entered by holding BOOT, press RST, release BOOT
  Serial.begin(115200);
  Serial.println("Code Starting");
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  //AS5600 Hall Encoder Setup
  Wire.begin(SDA, SCL);  //start wire with earlier defined pins
  
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

  total_encoder_counts = raw_counts + (4096 * revolutions);


  Serial.println("");
  Serial.print("Encoder Counts: ");
  Serial.println(total_encoder_counts);
  Serial.println("");  
  

//  if (digitalRead(SW3) == LOW) {
//    pos = pos + 300;
//    Serial.print("Pos increased to: ");
//    Serial.println(pos);
//  }
//
//  if (digitalRead(SW1) == LOW) {
//    pos = pos - 300;
//    Serial.print("pos decreased to: ");
//    Serial.println(pos);
//  }
//
//  if (digitalRead(SW2) == LOW) {
//    pos = 0;
//    Serial.println("pos reset");
//  }

  
  // Set values to send
//  myData.pos = pos;
  myData.pos = total_encoder_counts;
  
  // Send message via ESP-NOW (should probably schedule as doesnt need to be too fast
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result != ESP_OK) {
    Serial.println("Error sending the data");
  }
  delay(20);
}
