

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "secrets.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_KEY;
//const char* mqtt_server = "192.168.178.253";

const char* mqtt_server = "192.168.255.29";
//const char* mqtt_user = NULL;
//const char* mqtt_pass = NULL;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
	delay(10);
	Serial.println();
	Serial.print("Connecting to wifi\nSSID:");
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
	/*
	if ((char)payload[0] == '1') {
		digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
		// but actually the LED is on; this is because
		// it is acive low on the ESP-01)
	} else {
		digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
	}
	*/

}

boolean reconnect() {
	Serial.print("Attempting MQTT connection...");
	// Attempt to connect
	if (client.connect("DOOR-R","/doorR/status",1,true,"offline")) {
		Serial.println("connected");

		client.publish("/doorR/status", "online");
		client.subscribe("/doorR/opener");
	} else {
		Serial.print("failed, rc=");
		Serial.print(client.state());
	}

	return client.connected();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting K11-DOOR-R");
	//setup hardware
	setup_wifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
}

void loop() {
	static long lastReconnectAttempt = 0;
	static long lastMsg = 0;

  if(WiFi.status() != WL_CONNECTED) {
    client.disconnect();
    Serial.printf("Wifi not connected! status: %d\n", WiFi.status());
    WiFi.printDiag(Serial);
    delay(2000);
  }
  
	long now = millis();
  //Serial.print("connection Status:");
  //Serial.println(client.connected());
  
	if (!client.connected()) {
    if (now - lastReconnectAttempt > 5000) {
			lastReconnectAttempt = now;
			// Attempt to reconnect
			if (reconnect()) {
				lastReconnectAttempt = 0;
			}
		}
		//skip the rest of loop
		return;
	}

  client.loop();

	if (now - lastMsg > 2000) {
		lastMsg = now;

		char msg[20];
		ltoa(now,msg,10);
		Serial.print("Publish message: ");
		Serial.println(msg);
		client.publish("/doorR/counter", msg);
	}
}
