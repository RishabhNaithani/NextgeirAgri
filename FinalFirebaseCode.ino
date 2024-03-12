#include<ArduinoJson.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define RX2 16
#define TX2 17

StaticJsonDocument<256> doc;
StaticJsonDocument<256> incomdoc;
int rechumi, rectemp, recmoist;
int humi, temp , moistpercentage;
#define WIFI_SSID "IoT"
#define WIFI_PASSWORD "123456789"
#define API_KEY "AIzaSyCpXHPs4OS-FoEedjqiWJyfNMFtNHASok8"

#define DATABASE_URL "https://flutter-firebase-2ebb1-default-rtdb.firebaseio.com/" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;



void setup() {
  Serial.begin(115200);
  Serial2.begin(115200,SERIAL_8N1,RX2,TX2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to : ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Successfully connected to : ");
  Serial.println(WIFI_SSID);
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Serial.println();
  Serial.println("Sign up");
  Serial.print("Sign up new user");
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Serial.println("---------------");

  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {


  Serial.println("Data from the App");
if(Firebase.RTDB.getInt(&fbdo, "/Data/Soil Data/humidity"))
{
if (fbdo.dataType() == "int")
{
humi = fbdo.intData();
}

}
if(Firebase.RTDB.getInt(&fbdo, "/Data/Soil Data/temperature"))
{
if (fbdo.dataType() == "int")
{
temp = fbdo.intData();

}

}
if(Firebase.RTDB.getInt(&fbdo, "/Data/Soil Data/moisture"))
{
if (fbdo.dataType() == "int")
{
moistpercentage = fbdo.intData();

}

}
Serial.print("Humidity: ");
Serial.println(humi);
doc["humi"] = humi;
Serial.print("Temperature: ");
Serial.println(temp);
doc["temp"] = temp;
Serial.print("Moisture: ");
Serial.println(moistpercentage);
 doc["moistpercentage"] = moistpercentage;
  String jsondata;
  Serial.print("Sending data to the gateway! :");
  serializeJson(doc,jsondata);
  Serial.println(jsondata);
  size_t serializedSize = serializeJson(doc, Serial2);
  if (serializedSize > 0) {
    Serial.print("Serialization successful. Bytes written: ");
    Serial.println(serializedSize);
    delay(1000);
  }
  else{
    Serial.println("Error serializing data");
  }
    if(Serial2.available()){
  //data from the gateway is read here on the serial monitor 
  Serial.println("Data coming from Gateway ESP :");
  DeserializationError err = deserializeJson(incomdoc,Serial2);
  if(err == DeserializationError::Ok){
  rechumi = incomdoc["humi"].as<int>();
  Serial.print("Humidity :");
  Serial.println(rechumi);
  Firebase.RTDB.set(&fbdo, "/Data/Realtime Soil Data/humidity", rechumi);
  
  rectemp = incomdoc["temp"].as<int>();
  Serial.print("Temperature :");
  Serial.println(rectemp);
  Firebase.RTDB.set(&fbdo, "/Data/Realtime Soil Data/moisture", recmoist);
  
  recmoist= incomdoc["moistpercentage"].as<int>();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
  Serial.print("Moisture :");
  Serial.println(recmoist);
  Firebase.RTDB.set(&fbdo, "/Data/Realtime Soil Data/temperature", rectemp); 

  }
  else{
    Serial.println(err.c_str());
  }
  }


delay(2000);

}
