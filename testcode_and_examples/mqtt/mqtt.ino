#include <SimpleDHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
int pinDHT11 = 32;
SimpleDHT11 dht11(pinDHT11);

// Replace with your network credentials (STATION)
const char* ssid = "NETWORK NAME";
const char* password = "NETWORK PASSWORD";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(100)
char msg[MSG_BUFFER_SIZE];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement on channel inviamessaggi1234
      client.publish("inviamessaggi1234", "connesso");
      // and subscribe to channel ricevimessaggi1234
      client.subscribe("ricevimessaggi1234");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
  initWiFi();
 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() { 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    // read without samples.
    byte temperature = 0;
    byte humidity = 0;
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
      Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
      return;
    }
    Serial.println("Sending Data:");
    Serial.print("Temperatura: ");
    Serial.print((int)temperature); Serial.println(" *C"); 

    Serial.print("Umidita': ");
    Serial.print((int)humidity); Serial.println(" %");
    //publish an update of msg on channel inviamessaggi1234
    sprintf(msg, "Temperatura: %d *C\nUmidita': %d %%",int(temperature),int(humidity));
    client.publish("inviamessaggi1234", msg);
  } 
}