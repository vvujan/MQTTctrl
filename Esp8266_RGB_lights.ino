#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN            5
#define NUMPIXELS      15

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "ssid";
const char* password = "pass"; 
char* topic = "/leds";   
char* server = "Server address"; 
char message_buff[100];  
String msgData;
unsigned long oldPressTime = 0;
unsigned long lastClickTime = 0;
boolean oneClick = false;

int stringPosition;
String method;
int red; 
int green = 0; 
int blue = 0;
int delayInput;
int brigthness;
String message="\0";
String lastMessage= "\0";
bool check=false;


WiFiClient wifiClient;

// PubSubClient MQTT subscription function:
void callback(char* topic, byte* payload, unsigned int length) {
  int i = 0;
  Serial.println("Message arrived:  topic: " + String(topic));
  Serial.println("Length: " + String(length,DEC)); 
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  //Buffer to string conversion
  String msgData = String(message_buff);
  String topicData = (String(topic) + String(msgData));
  Serial.println("Payload: " + msgData);
  selectMode(msgData);
  }


PubSubClient client(server, 1883, callback, wifiClient);
  
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

String selectMode(String message){
  method = message.substring(0,message.indexOf(";"));

  if(method=="static"){
    colorPicker(message);
  }
  else if(method == "dynamic"){
    rainbow(message);
  }
  return method;
}


void colorPicker(String message){
 
  if(lastMessage!=message){
    lastMessage=message;
    check=true;
  }
    
   Serial.println(lastMessage);
   Serial.println(message);
  
  method = message.substring(0,message.indexOf(";"));
  stringPosition = message.indexOf(";");
  message.remove(0,stringPosition+1); 
  
  red = message.substring(0,message.indexOf(";")).toInt();
  stringPosition = message.indexOf(";");
  message.remove(0,stringPosition+1);
  
  green = message.substring(0,message.indexOf(";")).toInt();
  stringPosition = message.indexOf(";");
  message.remove(0,stringPosition+1); 

  blue = message.substring(0,message.indexOf(";")).toInt();
  stringPosition = message.indexOf(";");
  message.remove(0,stringPosition+1); 
  
  pixels.setBrightness(255);
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(red,green,blue));
  }
  pixels.show();
  
}

void rainbow(String message) {
  uint16_t i, j;
  if(lastMessage!=message){
    lastMessage=message;
    check=false;
  }
  
  method = message.substring(0,message.indexOf(";"));
  stringPosition = message.indexOf(";");
  message.remove(0,stringPosition+1); 
  
  delayInput = message.substring(0,message.indexOf(";")).toInt();
  stringPosition = message.indexOf(";");
  message.remove(0,stringPosition+1);

  brigthness = message.substring(0,message.indexOf(";")).toInt();
  stringPosition = message.indexOf(";");
  message.remove(0,stringPosition+1);

  pixels.setBrightness(brigthness);
  for(j=0; j<256; j++) {
    if(check==false){
      for(i=0; i<NUMPIXELS; i++) {
        pixels.setPixelColor(i, Wheel((i+j) & 255));
      }
      pixels.show();
      delay(delayInput);
      client.loop();
      if(msgData != message){    
        selectMode(msgData);
      } 
    }
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setup() {
  Serial.begin(115200);
  pixels.begin(); // This initializes the NeoPixel library.
  for (int i=0; i<NUMPIXELS; i++)
    pixels.setPixelColor(i, pixels.Color(0,50,0));
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid); 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
//  connection to broker script.
  if (client.connect("heartrtrc1")) {
    client.publish("/haptic/IO","Lights is up"); //MQTT publish to the topic "/haptic/IO"and msg "Haptic is up"
    client.subscribe(topic);
  }  
}

void loop() {
  if(client.connected()){
    client.loop();
    if(lastMessage !=  msgData)
      Serial.println(msgData); 
    selectMode(msgData);
    if(message=="\0")
      selectMode(lastMessage);
  }
  else{
    reconnect();
    
  }
}
