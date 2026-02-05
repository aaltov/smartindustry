// This example uses an ESP32 Development Board
// to connect to shiftr.io.
//
//
// Based on code by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <WiFi.h>
#include <MQTT.h>
#include "Ultrasonic.h"
Ultrasonic ultrasonic(D7);

////// Give database information here:
String myDB = "data_ml"; //unique database name
String myColl = "p_count_2"; //unique collection name
String myID = "robo"; //serial number, mac address or other unique identification for your device
//////

////// Give WiFi credentials here
const char ssid[] = "Robo_Device_net";
const char pass[] = "RoboGarage2@25";
//////

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0; // MQTT send timer
unsigned long lastMillis2 = 0; // pCount timer

int pCount = 0;
float sensor = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("ESP-32-eki2", "automaatio", "Z0od2PZF65jbtcXu")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("automaatio");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  client.begin("automaatio.cloud.shiftr.io", net);
  client.onMessage(messageReceived);

  connect();

}

void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

////// Here is the Person Counter part
////// This code polls continuously an IR proximity sensor,
////// and if it detects a person, it increases the pCount value.
////// After this, there is a 500 ms delay before the next person will be counted.
////// Persons are counted for 30 seconds and the total bumber of persons counted
////// is then sent to database

/*
if (millis() - lastMillis2 > 500) {
  lastMillis2 = millis();
  sensor = ultrasonic.MeasureInCentimeters();
  if(sensor < 100) pCount++;
  //Serial.println(sensor);
}
*/

sensor = ultrasonic.MeasureInCentimeters();
if(sensor < 100) {
  Serial.println(sensor);
  pCount++;
  delay(1000);
}


  // publish a message roughly every 30 seconds.
  if (millis() - lastMillis > 300000) {
    lastMillis = millis();

///// Here, a JSON message to be published is parsed.
///// first, we include database and ID information:
  String PubString = "{\"db_name\":\"" + myDB + "\"" + "," +
                      "\"coll_name\":\"" + myColl + "\"" + "," +
                      "\"id\":\"" + myID + "\"" + "," +

/////Then, the value of person counter:
                       "\"person count\":" + pCount +

///// Finally, the JSON is closed
                       "}";

///// The string is then published
    client.publish("automaatio", PubString);

///// Print pCount for debugging
    Serial.println(pCount);

    pCount = 0; // reset person counter
  }
}
