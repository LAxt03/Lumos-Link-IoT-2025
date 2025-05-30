
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// === Pins ===
const int ledPin = 14;
const int soundPin = 34;

const char* homessid = "****";
const char* homepassword = "****";


// === WLAN Zugangsdaten ===
const char* ssid = homessid;
const char* password = homepassword;

// === MQTT Broker ===
const char *mqtt_broker = "192.168.1.21";// "192.168.178.100";
const int mqtt_port = 1883;
const char *mqtt_username = "lumos";
const char *mqtt_password ="AAL_IoT";


// === Luminos Connection Data ===
const int partners_count = 3;
unsigned long partners[partners_count] = {0x00000001};

const unsigned long lumos_id = 0x00000002; //MAX:0xFFFFFFFF


WiFiClient espClient;
PubSubClient client(espClient);

// === Funktion: WLAN verbinden ===
void setup_wifi() {
    delay(100);
    Serial.println();
    Serial.print("Verbinde mit WLAN: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println(" WLAN verbunden!");
    Serial.print("IP Adresse: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    String message = String();
    for (int i = 0; i < length; i++) {
        message.concat((char) payload[i]);
    }
    Serial.println(message);
    analogWrite(ledPin, message.toInt());
    Serial.println("-----------------------");
}
//String((char) payload[i]).toInt()
int soundcounter = 0;
int measureSound(){
    long sound = 0;
    for(int i=0;i<7;i++){
        sound +=analogRead(soundPin);
    }

    sound = ((sound/7));
    Serial.print("Sound: ");
    Serial.println(sound);
    if(sound<220&&sound>80) {
        sound=0;
        soundcounter = 0;
    }
    else {
        soundcounter++;
    }
    if (soundcounter>1){
        return sound-sound/4;
    }
    else {
        return 5;
    }
}


void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    setup_wifi();
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = String(lumos_id,HEX);
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    Serial.println("checkpoint 1");

    for (int i = 0; i < partners_count; i++) {
        if (partners[i]!=0) {
            client.subscribe(String(partners[i],HEX).c_str());
            Serial.print("Subscrinbed to: ");
            Serial.println(String(partners[i],HEX).c_str());
        }
    }
}

void loop() {
    client.loop();
    client.publish(String(lumos_id,HEX).c_str(),std::to_string(measureSound()).c_str());
    delay(500);

}

