
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <Ticker.h>
#include <PubSubClient.h>


#define USE_SERIAL Serial

#define UPDATE_BLINK .1
#define CONNECTED_BLINK .25
#define INPUT_PIN 4



const char* ssid = "";
const char* password = "";
const char* mqtt_server = "192.168.86.101";


//MQTT Vars....
WiFiClient espClient;
PubSubClient client(espClient);


char GARAGE_LIGHT_ON[] = "{'garage_light': {'status': '1','location': 'center top garage'}}";
char GARAGE_LIGHT_OFF[] = "{'garage_light': {'status': '0','location': 'center top garage'}}";
char GARAGE_REGISTER_MQTT[] = "{'motion_event': {'event_type': 'registration','location': 'center top garage'}}";

boolean toggle = false;
String VERSION = "1.0";

Ticker blinker;
Ticker updateTicker;

ESP8266WiFiMulti WiFiMulti;
int led = LED_BUILTIN;

void blink();
void checkForUpdates();
void WiFiEvent(WiFiEvent_t event);
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();


// void checkForUpdates(){
//   // wait for WiFi connection
//   if((WiFi.status() == WL_CONNECTED)) {
//     String URL = "http://192.168.86.185:8266/update_light/update.bin";
//     //Serial.println("Trying Server: " + URL);
//     Serial.println(WiFi.localIP());
//       t_httpUpdate_return ret = ESPhttpUpdate.update(URL,VERSION);
//       //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");
//
//       switch(ret) {
//           case HTTP_UPDATE_FAILED:
//               USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
//               break;
//
//           case HTTP_UPDATE_NO_UPDATES:
//               USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES\n");
//               break;
//
//           case HTTP_UPDATE_OK:
//               USE_SERIAL.println("HTTP_UPDATE_OK\n");
//               break;
//       }
//   }
// }
//
// void blink(){
//   toggle = !toggle;
//   digitalWrite(led, toggle);
//
// }

void connectWifi();
void connectMQTT();
void monitorLightState();

void (*stateFunc)();


//******************************
//  Initial State 1:
//  WIFI Connection State
//******************************

void connectWifi(){
  USE_SERIAL.printf("Connecting To Wifi\n");
  //wifiClient.disconnect();
  WiFi.begin("SSID","password");
  //
  // delay(1000);
  while (WiFi.status() != WL_CONNECTED) {
    //Block this state until we get a connection to the wifi network
    USE_SERIAL.printf("Attempting to connect to network.... %d\n", WiFi.status());
    delay(1000);
    yield();
  }
  // Connected to wifi
  IPAddress ad;
  ad =  WiFi.localIP();
  USE_SERIAL.printf("Connected To Wifi: ");
  Serial.println(ad.toString());
  //WiFi.onEvent(WiFiEvent);
  stateFunc = connectMQTT;
}



//******************************
//  State 2:
//  connectMQTT Connection State
//******************************

void connectMQTT(){
  client.setServer(mqtt_server,1183);
  client.setCallback(mqttCallback);
  //Create a unique Id;
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);

  USE_SERIAL.printf("Connecting to MQTT server %s\n", mqtt_server);

  while(!client.connected()){
      Serial.print("Attempting MQTT connection... CONNECTED: ");
      Serial.print(client.connected());
      Serial.println();

      //Feed the watch dogs
      delay(1000);
      client.loop();
  }
  //register the listening topics
  client.subscribe("MOTION_EVENT_DEBUG");
  client.subscribe("CHANGE_UPDATER_CALLBACK");
  client.subscribe("UPDATE_FIRMWARE");

  //register the connection with the home automation server
  client.publish("HOME_AUTOMATION", GARAGE_REGISTER_MQTT);

  stateFunc = monitorLightState;
}


//******************************
//  State 3:
//  Monitor Light State
//******************************

void monitorLightState(){
  Serial.print("Monitor Light State Listening...\n");
  //Check to see we are still connected to wifi and mqtt before starting


}



//******************************
//  Initialization Of ESP
//
//******************************
void setup() {
    //Setup Initial values
    pinMode(led, OUTPUT);
    pinMode(INPUT_PIN, INPUT);
    //attachInterrupt(INPUT_PIN, setMotionDetectedFlag, RISING);
    randomSeed(micros());
    blinker.detach();
    USE_SERIAL.begin(115200);
    stateFunc = connectWifi;
}

void mqttCallback(char* topic, byte* payload, unsigned int length){
    USE_SERIAL.printf("Message Arrived: %s", topic);
}


//******************************
//  Loop
//  cycles though states
//******************************

void loop() {

  while(1){
    (*stateFunc)();
    delay(1000);

  }
}
