#include <WiFi.h>
#include <PubSubClient.h>
const char* ssid = "BandOfBrothers";
const char* password = "HOMEBARIBAL9";
const char* mqttServer = "192.168.0.242";
const int mqttPort = 1883;
const char* mqttUser = "EwoutDoms";
const char* mqttPassword = "SMOD3344";
WiFiClient espClient;
PubSubClient client(espClient);

////////////////////////////////////////////////////

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#define BOTtoken "7009974340:AAET5McU0Lr7IFZE-J-bs-U6rvgCctaCVi4"
#define CHAT_ID "7163672330"
WiFiClientSecure telegramclient;
UniversalTelegramBot bot(BOTtoken, telegramclient);
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

////////////////////////////////////////////////////

#include <ESP32Servo.h>
Servo myservo;

////////////////////////////////////////////////////

const int ldr = 34;
const int led = 19;
const int servo = 22;
int y = 0;
int hoi = 1;


//------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  ////////////////////////////////////////////////////

  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password);
  telegramclient.setCACert(TELEGRAM_CERTIFICATE_ROOT);  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Zoeken naar wifi");
  }
  Serial.println("geconecteerd met wifi netwerk");
  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    Serial.println("connecteren met MQTT");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Geconnecteerd met MQTT");
    } else {
      Serial.print("Gefaald met connecteren met MQTT ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "We kunnen beginnen", "");

  ////////////////////////////////////////////////////

  client.setCallback(callback);
  client.subscribe("brievenbus/slot");

  ////////////////////////////////////////////////////

  pinMode(ldr, INPUT);
  pinMode(led, OUTPUT);
  myservo.attach(servo);
  myservo.write(0);
}


//-------------------------------------------------------------------
void loop() {
  client.loop();
  ////////////////////////////////////////////////////

  int ldrwaarde = analogRead(ldr);
  Serial.print("ldr waarde: ");
  Serial.println(ldrwaarde);

  if (ldrwaarde <= 500) {
    y++;
    delay(10000);
    bot.sendMessage(CHAT_ID, "Brievenbus " + String(y) + " keer geopend", "");
    Serial.println("Brievenbus " + String(y) + " keer geopend");
    String ldrtekst = String(y);
    client.publish("brievenbus/huis/open", ldrtekst.c_str());
    Serial.println("Brievenbus geopend");
  }

  if (ldrwaarde > 500) {
    String ldrtekst = String(0);
    client.publish("brievenbus/huis/dicht", ldrtekst.c_str());
    Serial.println("Brievenbus gesloten");
  }

  ////////////////////////////////////////////////////

  Serial.println(" ");
  delay(2000);
}


////////////////////////////////////////////////////

void callback(char* topic, byte* payload, unsigned int length) {
  String bericht = "";

  if (String(topic) == "brievenbus/slot") {
    for (int i = 0; i < length; i++) {
      bericht += (char)payload[i];
    }
    if (bericht == "Servo 180") {
      myservo.write(180);
      digitalWrite(led, HIGH);
      Serial.println("Slot: open");

    } else if (bericht == "Servo 0") {
      myservo.write(0);
      digitalWrite(led, LOW);
      Serial.println("Slot: gesloten");
      
      y = 0;
      bot.sendMessage(CHAT_ID, "Herstart telling van openen brievenbus: " + String(y), "");
      Serial.println("Herstart telling van openen brievenbus: " + String(y));

      String ldrtekst = String(0);
      client.publish("brievenbus/huis/open", ldrtekst.c_str());
    }
  }
}
