#include <ESP8266WiFi.h>
#include <PubSubClient.h> // В библиотеке PubSubClient.h необходимо увеличить значения MQTT_KEEPALIVE минимум до 120 (можно больше)

// ОСНОВНЫЕ НАСТРОЙКИ
const char* wifiNetwork = "YourWiFiNetwork"; // Название сети WiFi
const char* wifiPassword = "YourWiFiPassword"; // Пароль сети WiFi
IPAddress   mqttServer(192, 168, 1, 100); // IP-адрес MQTT-брокера
const char* clientName = "NRG Meter v0.99"; // Имя клиента
const char* mqttUser = "YourMQTTBrokerUser"; // Имя пользователя для MQTT-брокера
const char* mqttPassword = "YourMQTTBrokerPassword"; // Пароль для MQTT-брокера
const char* mqttInTopic = "sensors/nrg/config"; // Топик для получения данных
const char *mqttOutTopic;
char impulseCount[3];

// КОНТАКТ ДАТЧИКА
int pinNumberTEMT6000 = A0;

// НАСТРОЙКИ
const int illuminanceThreshold = 10; // Порог срабатывания счетчика по уровню освещенности
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
PubSubClient client(mqttServer, 1883, callback, wifiConnection); // MQTT

// Инициализация
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
  if ((!client.connected()) & (impulseCounter > 100)) {
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
    itoa(impulseCounter, impulseCount, 10);
    client.publish(mqttOutTopic, impulseCount);
    sendReply = false;
    impulseCounter = 0;
    delay(5);
  }

  client.loop();
}
