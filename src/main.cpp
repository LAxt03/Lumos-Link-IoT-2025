
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// === Pins ===
const int lightSensPin = 14;
const int soundPin = 34;
const int ledPin= 12;

// === LED DATA ===
const int NUMPIXELS = 12;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ledPin, NEO_GRB + NEO_KHZ800);
unsigned int RGB_values[3] = {0,0,0};

const char* homessid = "***";
const char* homepassword = "***";



// === WLAN Zugangsdaten ===
const char* ssid = homessid;
const char* password = homepassword;//TODO: Aaron muss sich um die wifi maneger kümmer

// === MQTT Broker ===
const char *mqtt_broker = "192.168.178.100"; // eventuell auch über wifimaneger
const int mqtt_port = 1883;
const char *mqtt_username = "lumos";
const char *mqtt_password ="AAL_IoT";


// === Luminos Connection Data ===
const int partners_count = 3;
unsigned long partners[partners_count] = {0x00000001,NULL, 0x00000001}; // IDs der anderen lumose die Durch Wifimaneger gestellt werden, NH AARON????

const unsigned long lumos_id = 0x00000001; //MAX:0xFFFFFFFF

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

void callback(char *topic, byte *payload, unsigned int length) { //function die aufgerufen wird wenn auf den gefolgten MQTT kanal was gesendet wird
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
    for (int i = 0; i < partners_count; i++) { //TODO: Was wenn ein lumos nicht mehr sendet und der letzte wert nicht 0 war?
        if (strcmp(String(partners[i],HEX).c_str(), topic) == 0) {
            int value = message.toInt();
            if (value > 255) {
                RGB_values[i] = 255;
            }
            else {
                RGB_values[i] = value;
            }
        }
    }
}
int soundcounter = 0; // counter wie oft hintereinander ein sound gemessen wird um einzelne Ausschläge zu ignorieren
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

// === LED functions ===
void draw_led(int R,int G, int B) {
    for(int i=0; i<NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(R, G, B));
    }
    pixels.show();

}


void setup() {
    //draw_led(255, 255, 255);
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
    draw_led(RGB_values[0],RGB_values[1],RGB_values[2]);
    delay(500);

}

