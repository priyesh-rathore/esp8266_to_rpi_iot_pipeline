#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int trigPin = 12;
const int echoPin = 14;

String ON = "ON";
String OFF = "OFF";

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

char ssid[] = "Airtel-MyWiFi-AMF-311WW-F768";
char pass[] = "13b861f0";

// MQTT Broker
const char *mqtt_broker = "192.168.1.200";
const char *topic_pub_1 = "UT_IOT/distance";
const char *topic_pub_2 = "UT_IOT/led";
const char *topic_sub = "UT_IOT/esp1/led";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

long duration;
float distanceCm;
float distanceInch;
String dis_to_publish;

int hook = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length);

void setup() {
  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
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
    String client_id = "esp8266_1-client-priyesh";
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
  client.publish(topic_pub_1, "hello MQTT");
  client.subscribe(topic_sub);
}

long int now = millis();

void loop() {

  long int then = millis();
  
  client.loop();

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY / 2;

  // Prints the distance on the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  dis_to_publish = String(distanceCm, 1);// using a float and the decimal places


  if(then - now >= 2500){
    now = millis();
    client.publish(topic_pub_1, String(distanceCm).c_str());
    }


  int sensorValue = analogRead(A0);   // read the input on analog pin 0

  float voltage = sensorValue * (5.0 / 1023.0);   // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)

  Serial.println(voltage);   // print out the value you read

  client.publish(topic_pub_2, String(voltage).c_str());

  if (!client.connected()) {
    String client_id = "esp8266_1-client-priyesh";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
      client.subscribe(topic_sub);
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

  if (String(topic) == "UT_IOT/esp1/led") {
    //Serial.println("Topic check executed successfully!");
    if (rx_msg == ON) {
      Serial.println("Switching ON");
      digitalWrite(LED_BUILTIN, LOW);
    }
    else if (rx_msg == OFF) {
      Serial.println("Switching OFF");
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }

}
