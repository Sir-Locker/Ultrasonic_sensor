#include <esp_now.h>
#include <WiFi.h>
#include "Ultrasonic.h"

// REPLACE WITH YOUR ESP RECEIVER'S MAC ADDRESS
uint8_t servoAddress[] = {0x24, 0x6F, 0x28, 0x28, 0x17, 0x1C};

bool compareMac(const uint8_t * a,const uint8_t * b){
  for(int i=0;i<6;i++){
    if(a[i]!=b[i])
      return false;    
  }
  return true;
}

typedef struct ultrasonic_send {
  int stateUltra; // 1 = ประตูปิด, 0 = ประตูเปิด
} ultrasonic_send;

ultrasonic_send test;

typedef struct servo_struct{
  int servo_status;//1 = lock, 0 = unlock
} servo_struct;

// Create a struct_message called myData
servo_struct myData;
servo_struct servoBoard;

esp_now_peer_info_t peerInfo;

int c = 0;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  if(compareMac(mac_addr,servoAddress)){
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.printf("--------From Choon : servo---------\n");
    servoBoard.servo_status = myData.servo_status;
    Serial.printf("status: %d \n", servoBoard.servo_status);
    Serial.printf("---------------------------------\n");
    Serial.println();
  }
}

Ultrasonic ultrasonic(22);
void setup()
{
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
 
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
   
  // register peer
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // register first peer  
  memcpy(peerInfo.peer_addr, servoAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
void loop()
{

  long RangeInInches;
  long RangeInCentimeters;

  Serial.println("The distance to obstacles in front is: ");

  RangeInCentimeters = ultrasonic.MeasureInCentimeters(); // two measurements should keep an interval
  Serial.print(RangeInCentimeters);//0~400cm
  Serial.println(" cm");
  if(RangeInCentimeters<5){
    c = c+1;
    if(c > 2){
      test.stateUltra = 1;
    }
  }
  else{
    test.stateUltra = 0;
    c = 0;
  }
  esp_err_t result = esp_now_send(servoAddress, (uint8_t *) &test, sizeof(ultrasonic_send));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
 

  int recv_servoBoard = servoBoard.servo_status;
  Serial.printf("recv_servoBoard status: %d \n", recv_servoBoard);
  Serial.println();
  delay(1000);
}
