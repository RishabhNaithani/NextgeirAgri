#include <WiFi.h>
#include <esp_now.h>
#include<ArduinoJson.h>
#include <ESP32_Servo.h>
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
StaticJsonDocument<256> incomdoc;
StaticJsonDocument<256> send_doc;

#define relay1 25
#define relay2 26

#define RXD2 16
#define TXD2 17
Servo valve1;
Servo valve2;
Servo valve3; 
typedef struct datafromNetwork {
  int temperature;
  int humidity;
  int moisture;
  int board ;
  int state;
} datafromNetwork;
datafromNetwork recievedData;


typedef struct datatoNetwork {
  int temperature;
  int humidity;
  int moisture;
  int board = 0;
} datatoNetwork;
datatoNetwork dataTosend;



void checkRelay(int b,int s){
  switch(b){
    case 1:
          if(s <9 && s>=0){
            valve1.write(0);
            digitalWrite(relay1,HIGH);
          }
          else if(s<25 && s>10){
            valve1.write(22);
            digitalWrite(relay1,LOW);
          }
          else if(s<50 && s>26){
            valve1.write(45);
            digitalWrite(relay1,LOW);
          }
          else if(s<75 && s>51){
            valve1.write(67);
            digitalWrite(relay1,LOW);
          }
          else if(s<100 && s>76){
            valve1.write(90);
            digitalWrite(relay1,LOW);
          }

    break;
    case 2:
    if(s <9 && s>=0){
            valve2.write(0);
            digitalWrite(relay2,HIGH);
          }
          else if(s<25 && s>10){
            valve2.write(22);
            digitalWrite(relay2,LOW);
          }
          else if(s<50 && s>26){
            valve2.write(45);
            digitalWrite(relay2,LOW);
          }
          else if(s<75 && s>51){
            valve2.write(67);
            digitalWrite(relay2,LOW);
          }
          else if(s<100 && s>76){
            valve2.write(90);
            digitalWrite(relay2,LOW);
          }
    break;
  }
}


void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recievedData, incomingData, sizeof(recievedData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Temperature : ");
  Serial.println(recievedData.temperature);
  Serial.print("Moisture : ");
  Serial.println(recievedData.moisture);
  Serial.print("Humidity : ");
  Serial.println(recievedData.humidity);
  Serial.print("Board : ");
  Serial.println(recievedData.board);
  Serial.print("state : ");
  Serial.println(recievedData.state);
if(recievedData.board!=0){
  Serial.println("data echo");
  send_doc["humi"] = recievedData.humidity;
  send_doc["moistpercentage"] = recievedData.moisture;
  send_doc["temp"] = recievedData.temperature;
  String jsondata;
  serializeJson(send_doc,jsondata);
  Serial.println(jsondata);
  serializeJson(send_doc,Serial2);
  checkRelay(recievedData.board,recievedData.state);
  }
  else{
    esp_err_t result = esp_now_send(broadcastAddress,(uint8_t *) &recievedData,sizeof(recievedData));
    if(result == ESP_OK){
    Serial.println("Forwarding the data");
    // Serial.println(recievedData.board);
  }
  else{
    Serial.print("failed to forward data");
  }
  }
}



void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}





void setup() {

  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  valve1.attach(13);  
  valve2.attach(12);//not working right now 
  valve3.attach(32);
  valve3.write(0);
  valve2.write(0);
  valve1.write(0);
  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);


  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int i = 0; i < 6; i++)
  {
    peerInfo.peer_addr[i] = (uint8_t) broadcastAddress[i];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}


void loop()
{

  if(Serial2.available()){
    DeserializationError err = deserializeJson(incomdoc,Serial2);
  if(err == DeserializationError::Ok){
  dataTosend.humidity = incomdoc["humi"].as<int>();
  Serial.print("Humidity :");
  Serial.println(dataTosend.humidity);
  
  dataTosend.temperature = incomdoc["temp"].as<int>();
  Serial.print("Temperature :");
  Serial.println(dataTosend.temperature);
  
  dataTosend.moisture = incomdoc["moistpercentage"].as<int>();
  Serial.print("Moisture :");
  Serial.println(dataTosend.moisture);
  }
  else{
    Serial.println(err.c_str());
  } 
    esp_err_t result = esp_now_send(broadcastAddress,(uint8_t *) &dataTosend,sizeof(dataTosend));
    if(result == ESP_OK){
    Serial.println("Sending data to the network!");
    // Serial.println(recievedData.board);
  }
  else{
    Serial.print("failed to send data to the network");
  }
  }

}