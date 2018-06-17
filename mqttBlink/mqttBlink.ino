/*************************
I used Adafruit examples and a lot
of help from Arduino forums to write this
code. A lot of the code comes directly
from Adafruit, but it has a few modifications
to meet my needs. 
*************************/


/*************************** The LED stuff ****************************/
#define LED 13

//the current state
int current = 1;

unsigned long timer;

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
 
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "YourWifiName"
#define WLAN_PASS       "YourWifiPassword"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                        // use 8883 for SSL
#define AIO_USERNAME    "adafruitUserName"          //your adafruit.io user name
#define AIO_KEY         "adafruitKey"               //your adafruit key

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  // Setup MQTT subscription for onoff
  mqtt.subscribe(&onoffbutton);  
  timer = millis();
}


void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
 
  Adafruit_MQTT_Subscribe *subscription;
  Serial.println("Waiting for data");
  while ((subscription = mqtt.readSubscription(5000))) {
    // Check if its the onoff button feed
    Serial.println("Recieved data");
    if (subscription == &onoffbutton) {
      char *value = (char *)&onoffbutton.lastread;
      current = atoi(value);
      Serial.print(F("On-Off button: "));
      Serial.println(current);
      
      if (current == 0) {
        digitalWrite(LED, LOW);
      }
        Serial.println("I am turing off");
      }
      if (current == 1) {
        digitalWrite(LED, HIGH);
        Serial.println("I am turing on");
      }
      

  }
  
  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
