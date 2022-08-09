#include <Servo.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

char ssid[] = "Airtel-MyWiFi-AMF-311WW-F768";
char pass[] = "13b861f0";

// MQTT Broker
const char *mqtt_broker = "192.168.1.200";
const char *topic_pub = "UT_IOT/pot";
const char *topic_sub_1 = "UT_IOT/servo_1";
const char *topic_sub_2 = "UT_IOT/servo_2";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length);

Servo servo_1;
Servo servo_2;

int s1 = 0;
int s2 = 0;

void setup() {
  Serial.begin(115200); // Starts the serial communication

  servo_1.attach(5); //D1
  servo_1.write(0);

  servo_2.attach(14); //D5
  servo_2.write(0);

  Serial.println();

  WiFi.begin(ssid, pass);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp8266_2-client-priyesh";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  // publish and subscribe
  client.publish(topic_pub, "hello from MQTT ESP2");
  client.subscribe(topic_sub_1);
  client.subscribe(topic_sub_2);
}

void loop() {
  client.loop();

servo_1.write(s1);
servo_2.write(s2);
  
  int sensorValue = analogRead(A0);   // read the input on analog pin 0

  float voltage = sensorValue * (24.0 / 1023.0);   // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 24V)

  //Serial.println(voltage);   // print out the value you read

  client.publish(topic_pub, String(voltage).c_str());

  if (!client.connected()) {
    String client_id = "esp8266_2-client-priyesh";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
      client.subscribe(topic_sub_1);
      client.subscribe(topic_sub_2);
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");

  String rx_msg = "";

  for (int i = 0; i < length; i++) {
    //Serial.print((char) payload[i]);
    rx_msg += (char)payload[i];
  }
  Serial.println(rx_msg);
  Serial.println("-----------------------");

  if (String(topic) == "UT_IOT/servo_1") {
        s1 = rx_msg.toInt();
  }
  if (String(topic) == "UT_IOT/servo_2") {
        s2 = rx_msg.toInt();
  }

}
