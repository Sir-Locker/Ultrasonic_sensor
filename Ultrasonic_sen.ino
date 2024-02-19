#include <esp_now.h>
#include <WiFi.h>
#include "Ultrasonic.h"

// REPLACE WITH YOUR ESP RECEIVER'S MAC ADDRESS
uint8_t broadcastAddress1[] = {0x3C, 0x61, 0x05, 0x03, 0xCA, 0x04};

typedef struct test_struct {
  int sendData;
} test_struct;

test_struct test;

typedef struct send_mode{
  int statuss; 
}send_mode;

// Create a struct_message called myData
send_mode myData;

send_mode board1;

esp_now_peer_info_t peerInfo;

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
  memcpy(&myData, incomingData, sizeof(myData));
  // Update the structures with the new incoming data
  board1.statuss = myData.statuss;
  Serial.printf("status: %d \n", board1.statuss);
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
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
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
    test.sendData = 1;
  }
  else{
    test.sendData = 0;
  }
 
  esp_err_t result = esp_now_send(broadcastAddress1, (uint8_t *) &test, sizeof(test_struct));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  int recv_board1 = board1.statuss;
  Serial.printf("recv_board1 status: %d \n", recv_board1);
  Serial.println();
  delay(1000);
}
