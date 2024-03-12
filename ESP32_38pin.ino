
#include <esp_now.h>
#include <DHT.h>
#include <WiFi.h>

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define DHT11PIN 32          
#define soilPin 33
#define highPin 15
#define highPinCap 5
#define lowPinCap 18
#define redLed 12
#define greenLed 14
DHT dht(DHT11PIN, DHT11);

const int water = 1080;
const int air = 2900;
int mainmoist = 100;

typedef struct struct_message {
  int temperature;
  int humidity;
  int moisture;
  int board = 3;
  int state;
} struct_message;
struct_message mydata;                   


typedef struct masterData {
  int temperature;
  int humidity;
  int moisture;
  int board;
} masterData;
masterData recievedData;


void blink(int times,int LED){
  int i =0;
  while(times>=i){
    digitalWrite(LED,HIGH);
    delay(200);
    digitalWrite(LED,LOW);
    delay(200);
    i++;
  }
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
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
    if(recievedData.board == 0){
      Serial.println("Receiving data from APP");
      mainmoist = recievedData.moisture;
    }
    blink(3,redLed);
    esp_err_t resultBroad = esp_now_send(broadcastAddress, (uint8_t *) &recievedData, sizeof(recievedData));
    if(resultBroad == ESP_OK){
      
      Serial.print("forwarded board number: ");
      Serial.println(recievedData.board);
    }
    else{
      Serial.println("failed to forward data");
    }
    delay(2000);
  }

void setup() {
  
  dht.begin();
  Serial.begin(115200);
  pinMode(redLed,OUTPUT);
  pinMode(greenLed, OUTPUT);
  digitalWrite(redLed,LOW);
  digitalWrite(greenLed,LOW);
  pinMode(highPin,OUTPUT);
  pinMode(highPinCap,OUTPUT);
  pinMode(lowPinCap,OUTPUT);
  pinMode(DHT11PIN, INPUT);
  pinMode(soilPin,INPUT);
  digitalWrite(highPin,HIGH);
  digitalWrite(highPinCap,HIGH);
  digitalWrite(lowPinCap,LOW);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);            
  esp_now_register_recv_cb(OnDataRecv);
 esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int i = 0; i < 6; i++ )
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

int calc(int x, int y, int z){
  int f = (0.17*x)+(0.79*y)+(0.04*z);
  Serial.print("valve for f: ");
  f = map(f,135,55,100,0);
  Serial.println(f);
  return f;
}

void loop() {
   Serial.print("New mainmoist value from the app : ");
  Serial.println(mainmoist);                                       
  int temp = dht.readTemperature();
  int humi = dht.readHumidity();
  int soil = analogRead(soilPin);
  Serial.println(soil);
  Serial.println("mainmoist : ");
  Serial.println(mainmoist);
  int moist = map(soil,air,water,0,100); 
  int moist = map(soil,air,water,650,250);
  Serial.print("Temperature : ");
  Serial.println(temp);
  Serial.print("Humidity : ");
  Serial.println(humi);
  Serial.print("Moisture : "); 
  Serial.println(moist);
  mydata.temperature = temp;
  mydata.humidity = humi;
  mydata.moisture = moist;
  mydata.state = calc(moist,temp,humi);
  Serial.print("Sending state value to the gateway: ");
  Serial.println(mydata.state);
  // if(mainmoist>moist){
  //   mydata.state = 1;
  // }
  // else {    
  //   mydata.state = 0;
  // }
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &mydata, sizeof(mydata));
   if (result == ESP_OK) {
    Serial.println("Sent with success");
    delay(1000);
    blink(3,greenLed);
  }
  else {
    Serial.println("Error sending the data");
  }
}