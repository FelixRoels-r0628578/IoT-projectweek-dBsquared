
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <EasyNTPClient.h>
#include <MCP3008.h>

//Watson IoT connection details
#define MQTT_HOST "a29i0f.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:a29i0f:ESP8266:sensorthing"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "QPlCqLj!mX*yOG&99f"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/update/fmt/json"
// Sensors
#define CS_PIN D8
#define CLOCK_PIN D5
#define MOSI_PIN D7
#define MISO_PIN D6

// Update these with values suitable for your network.
const char* ssid = "ucll-projectweek-IoT";
const char* password = "Foo4aiHa";
const char* mqtt_server = MQTT_HOST;
const int button = 0;
int buttonState = 0;

//Sensors
MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
int gemDb;
int gemPir;
int gemCo;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

WiFiUDP udp;
EasyNTPClient  ntpClient(udp, "time.google.com", 7200);

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // open serial port
  ntpClient.getUnixTime();
 gemDb =0;
 gemPir=0;
 gemCo=0;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
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
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC_DISPLAY);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  // Sensor data
   for(int i = 0; i < 100; i++){
 gemDb=gemDb+adc.readADC(0);
 gemPir = gemPir + digitalRead(D2);
 gemCo=gemCo + analogRead(A0);
 
  delay(100);
 }
 
 gemDb = gemDb/100;
 int valPir = gemPir;
 int valCo =gemCo/100;
 int dbNum = 0;
 bool movement = false;
 if(gemPir >=30) {
  movement=true;
 }
 if(gemDb <= 15){
  dbNum = 0;
 }
 if(15 < gemDb && gemDb<= 30){
    dbNum =2;
 }
  if(30 < gemDb && gemDb<= 45){
    dbNum =3;
 }
  if(45 < gemDb && gemDb<= 60){
    dbNum =4;
 }
  if(60 < gemDb && gemDb<= 75){
    dbNum =5;
 }
  if(75 < gemDb && gemDb<= 90){
    dbNum =6;
 }
  if(90 < gemDb && gemDb<= 105){
    dbNum =7;
 }
  if(105 < gemDb && gemDb<= 120){
    dbNum =8;
 } 
 if(120 < gemDb && gemDb<= 135){
    dbNum =9;
 } 
 if( gemDb > 135){
    dbNum =10;
 }
 Serial.println(gemDb); 
 gemDb =0;
 gemPir=0;
 gemCo=0;
 String moves ="";
 if(movement == 0){
  moves = "false";
 }
 else{moves="true";}
//
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Serial.println(adc.readADC(0));
//payload to json to IBM Watson
    String payload = "{";
    payload += "\"timestamp\":\""; payload += ntpClient.getUnixTime(); payload += "\",";
    payload += "\"noise\":"; payload += dbNum; payload += ",";                           
    payload += "\"movement\":"; payload += moves; payload += ",";                        
    payload += "\"CO\":"; payload += valCo; payload += ",";
    payload += "\"location\":\""; payload += "C004\"";                          
    payload += "}";
    Serial.println(payload);

    if (client.publish(MQTT_TOPIC, (char*) payload.c_str())) {
      Serial.println("Publish ok");
    } else {
      Serial.println("Publish failed");
    } 
}
