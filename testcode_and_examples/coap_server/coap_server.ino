#include <SimpleDHT.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple-observe.h>

// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
int pinDHT11 = 32;  //or whatever connected pin number 
SimpleDHT11 dht11(pinDHT11);

//initialize DHT variables
byte temperature = 0;
byte humidity = 0;
int err = SimpleDHTErrSuccess;

//initialize time-tracking variables
unsigned long currentTime = 0;
unsigned long previousTime = 0; 
unsigned long previousObservationTime = 0;

// Replace with your network credentials (STATION)
const char* ssid = "NETWORK NAME";
const char* password = "NETWORK PASSWORD";


// CoAP server endpoint url callback
void callback_data(CoapPacket &packet, IPAddress ip, int port);

// UDP and CoAP class
// other initialize is "Coap coap(Udp, 512);"
// 2nd default parameter is COAP_BUF_MAX_SIZE(default:128)
// For UDP fragmentation, it is good to set the maximum under
// 1280byte when using the internet connection.
WiFiUDP udp;
Coap coap(udp);

// CoAP server endpoint URL
void callback_data(CoapPacket &packet, IPAddress ip, int port) {
  
  // some data print for debugging purpose
  Serial.println("Sending Data:");
  Serial.print("Temperatura: ");
  Serial.print((int)temperature); Serial.println(" *C"); 
  Serial.print("Umidita': ");
  Serial.print((int)humidity); Serial.println(" %");
  
  // read message
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  // p is received payload
  String message(p);
  // handle the message if necessary
  Serial.println(p);

  // prepare data to send in response
  char datastring[100];
  sprintf(datastring, "Temperatura: %d *C\nUmidita': %d %%",int(temperature),int(humidity));
  
  //scan packet options looking for observe
  int if_observed = 0;
  for (int i = 0; i < packet.optionnum; i++) {
      if (packet.options[i].number == COAP_OBSERVE) if_observed = 1;
  }
  //if found observe option and observer list not full, send observe response
  if ((if_observed == 1) && (coap.notMaxObservers()))  
    coap.sendObserveResponse(ip, port, packet.messageid, datastring, strlen(datastring), COAP_CONTENT, COAP_TEXT_PLAIN, packet.token, packet.tokenlen);
  //else send normal response
  else  
    coap.sendResponse(ip, port, packet.messageid, datastring, strlen(datastring), COAP_CONTENT, COAP_TEXT_PLAIN, packet.token, packet.tokenlen);
}

// connect to wifi router and print local IP
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
 
  // add server url endpoints.
  // can add multiple endpoint urls.
  // exp) coap.server(callback_switch, "switch");
  //      coap.server(callback_env, "env/temp");
  //      coap.server(callback_env, "env/humidity");
  coap.server(callback_data, "data");

  // start coap server/client on standard port 5683
  coap.start();
  Serial.println("Setup CoAP Server: OK");

  // read sensor to initialize DHT data
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
    return;
  }
}

void loop() { 
  // OPTIONAL: set delay to prevent hardware-related errors
  delay(500);

  //read data from sensor every 5 seconds
  currentTime = millis();
  if ((currentTime - previousTime) > 5000) {
      previousTime = currentTime;
      if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
        Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
        Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
        return;
      }
  }

  //scan for incoming packets to manage
  coap.loop();  

  //send notifications to all observers every 10 seconds
  if ((currentTime - previousObservationTime) > 10000) {
      previousObservationTime = currentTime;
      // prepare data to send in response
      char data_to_notify[100];
      sprintf(data_to_notify, "Temperatura: %d *C\nUmidita': %d %%",int(temperature),int(humidity));
      coap.sendAllNotifications("data", data_to_notify, strlen(data_to_notify), COAP_TEXT_PLAIN);
  }

}
