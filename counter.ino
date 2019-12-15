#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* wifiNetwork = "NetworkName"; 
const char* wifiPassword = "NetworkPassword"; 
IPAddress   mqttServer(192, 168, 1, 100); 
const char* clientName = "NRG Meter v0.99";
const char* mqttUser = "mqttBrokerUser";
const char* mqttPassword = "mqttBrokerPassword";
const char* mqttInTopic = "sensors/nrg/config"; 
const char *mqttOutTopic;
char impulsesValue[2];

int pinNumberTEMT6000 = A0;

const int illuminanceThreshold = 10;
unsigned int impulseCounter = 0;
boolean impulseDetected = false;
boolean sendReply = false;
String topicName;

void callback(char* topic, byte* payload, unsigned int length)
{
  payload[length] = '\0';
  topicName = String((char*)payload); 
  mqttOutTopic = topicName.c_str();
  sendReply = true;
}

WiFiClient wifiConnection; // WiFi
PubSubClient client(mqttServer, 1883, callback, wifiConnection);

void setup() {
  pinMode(pinNumberTEMT6000, INPUT);
  WiFi.softAPdisconnect(true);
  WiFi.begin(wifiNetwork, wifiPassword);
}

void connect()
{

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }
  while (!client.connect(clientName, mqttUser, mqttPassword)) {
    delay(50);
  }
  client.subscribe(mqttInTopic); 
}

void loop()
{
  if (!client.connected()) {
    connect();
  }

  int illuminanceValue = analogRead(pinNumberTEMT6000);
  delay(5);

  if ((illuminanceValue > illuminanceThreshold) & (impulseDetected == false)) {
    impulseDetected = true;
    delay(5);
  }

  if ((illuminanceValue < illuminanceThreshold) & (impulseDetected == true)) {
    impulseDetected = false;
    impulseCounter ++;
    delay(5);
  }

  if (sendReply == true) {
    itoa(impulseCounter, impulsesValue, 10);
    client.publish(mqttOutTopic, impulsesValue);
    sendReply = false;
    impulseCounter = 0;
    delay(5);
  }

  client.loop();
}
