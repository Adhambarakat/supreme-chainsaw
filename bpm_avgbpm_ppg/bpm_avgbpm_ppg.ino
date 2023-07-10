#include <heartRate.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <TZ.h>
#include <FS.h>
#include <LittleFS.h>
#include <CertStoreBearSSL.h>

// Update these with values suitable for your network.
const char* ssid = "Orange-AKHWE";
const char* password = "akwe1603";
const char* mqtt_server = "7ea331cfad864375a4a189652c9375e0.s1.eu.hivemq.cloud";

// A single, global CertStore which can be used by all connections.
// Needs to stay live the entire time any of the WiFiClientBearSSLs
// are present.
BearSSL::CertStore certStore;


WiFiClientSecure espClient;
PubSubClient * client;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
int value = 0;

 MAX30105 sensor;

 int beats[25];
 float beat = 0;
 int avgbpm = 0;
 int i = 0;
 int arrayindex =0; 
 int x =0;

 char bpm_out[10];
 char avgbpm_out[10];
 char ppg_out[10];

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void setDateTime() {
  // You can use your own timezone, but the exact time is not used at all.
  // Only the date is needed for validating the certificates.
  configTime(TZ_Europe_Berlin, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(100);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if the first character is present
  if ((char)payload[0] != NULL) {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  } else {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  }
}


void reconnect() {
  // Loop until we’re reconnected
  while (!client->connected()) {
    Serial.print("Attempting MQTT connection…");
    String clientId = "ESP8266Client - MyClient";
    // Attempt to connect
    // Insert your password
    if (client->connect(clientId.c_str(), "ahmed", "Ahmed1603")) {
      Serial.println("connected");
      // client->publish("testTopic", int_out);
      // Once connected, publish an announcement…
      //   client->publish("testTopic", "hello world");
      // … and resubscribe
      //   client->subscribe("testTopic");
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client->state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  LittleFS.begin();
  setup_wifi();
  setDateTime();

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output

  // you can use the insecure mode, when you want to avoid the certificates
  //espclient->setInsecure();

  int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  Serial.printf("Number of CA certs read: %d\n", numCerts);
  if (numCerts == 0) {
    Serial.printf("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
    return; // Can't connect to anything w/o certs!
  }

  BearSSL::WiFiClientSecure *bear = new BearSSL::WiFiClientSecure();
  // Integrate the cert store with this connection
  bear->setCertStore(&certStore);

  client = new PubSubClient(*bear);

  client->setServer(mqtt_server, 8883);
  client->setCallback(callback);
  
  sensor.begin(Wire , I2C_SPEED_FAST);
  sensor.setup();
}

void loop() {
  if (!client->connected()) {
    reconnect();
  }
  client->loop();
  
  int IR = sensor.getIR();

  if(checkForBeat(IR) == true){
    Serial.println("beat detected");
    float delta = millis()- beat;
    beat = millis();
    int bpm = 60/(delta/1000);
    beats[arrayindex++] = bpm; //adding values to the array
    for(i=0; i<25; i=i+1)
      avgbpm+=beats[i];
      avgbpm/=i+1;

    Serial.print("BPM ="); 
    Serial.println(bpm); 
    Serial.print("G=");
    Serial.println(sensor.getGreen());

    sprintf(bpm_out, "%d", bpm);
    sprintf(ppg_out, "%d", sensor.getGreen());

    client->publish("bpm",bpm_out);
    client->publish("ppg",ppg_out);
    
    if(i == 24){
    Serial.print("avgbpm =");
    Serial.println(avgbpm,10);
    
    sprintf(avgbpm_out, "%d",avgbpm);

    client->publish("avgbpm",avgbpm_out);
    }

    }

  }
