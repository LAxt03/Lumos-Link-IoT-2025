
#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h> // Für WLAN-Konfiguration
#include <Preferences.h> // Für dauerhafte Speicherung
#include <Adafruit_NeoPixel.h>
Preferences prefs;
// === Pins ===
const int lightSensPin = 33;
const int soundPin = 34;
const int ledPin = 12;
const int turn_off_button = 14; // muss ein pin mit PULL-UP-RESISTOR sein
const int reset_button =4;     // muss ein pin mit PULL-UP-RESISTOR sein

// === LED DATA ===
const int NUMPIXELS = 12;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ledPin, NEO_GRB + NEO_KHZ800);
unsigned int RGB_values[3] = {0, 0, 0};


// Wifi Manager Parameter
char mqtt_broker[40];
char mqtt_port[6];
char mqtt_username[32];
char mqtt_password[32];
unsigned long lamp1 = 0;
unsigned long lamp2 = 0;
unsigned long lamp3 = 0;

// === Luminos Connection Data ===
const int partners_count = 3;
unsigned long partners[partners_count]; // IDs der anderen lumose die Durch Wifimaneger gestellt werden

const unsigned long lumos_id = 0x00000001; // MAX:0xFFFFFFFF

// WiFiManager
WiFiManager wm;

WiFiClient espClient;
PubSubClient client(espClient);

void saveConfig()
{
    prefs.begin("mqtt", false);
    prefs.putString("broker", mqtt_broker);
    prefs.putString("port", mqtt_port);
    prefs.putString("lamp1", String(lamp1, HEX));
    prefs.putString("lamp2", String(lamp2, HEX));
    prefs.putString("lamp3", String(lamp3, HEX));
    prefs.putString("mqtt username", mqtt_username);
    prefs.putString("mqtt password", mqtt_password);
    prefs.end();
    Serial.println("[INFO] Konfiguration gespeichert");
}

// Lädt die Configuration aus dem NVS Speicher
void loadConfig()
{
    prefs.begin("mqtt", true);
    String b = prefs.getString("broker", "");
    String p = prefs.getString("port", "");
    String u = prefs.getString("mqtt username", "");
    String pw = prefs.getString("mqtt password", "");

    lamp1 = strtoul(prefs.getString("lamp1", "0").c_str(), NULL, 16);
    lamp2 = strtoul(prefs.getString("lamp2", "0").c_str(), NULL, 16);
    lamp3 = strtoul(prefs.getString("lamp3", "0").c_str(), NULL, 16);

    partners[0] = lamp1;
    partners[1] = lamp2;
    partners[2] = lamp3;

    prefs.end();

    b.toCharArray(mqtt_broker, sizeof(mqtt_broker));
    p.toCharArray(mqtt_port, sizeof(mqtt_port));
    u.toCharArray(mqtt_username, sizeof(mqtt_username));
    pw.toCharArray(mqtt_password, sizeof(mqtt_password));

    Serial.println("[INFO] Geladene Konfiguration:");
    Serial.printf("MQTT Broker: %s\n", mqtt_broker);
    Serial.printf("MQTT Port: %s\n", mqtt_port);
    Serial.printf("Lampe1: %08lX\n", lamp1);
    Serial.printf("Lampe2: %08lX\n", lamp2);
    Serial.printf("Lampe3: %08lX\n", lamp3);
    Serial.printf("MQTT Username: %s\n", mqtt_username);
    // Serial.printf("MQTT Password: %s\n", mqtt_password); // Passwort nicht ausgeben
}

// Config Portal starten
void startConfigPortal()
{
    //Animation Start
    Serial.println("[INFO] Starte Konfigurations-Portal...");

    // Custom Parameter anlegen
    WiFiManagerParameter custom_mqtt_broker("broker", "MQTT Broker IP", mqtt_broker, 40);
    WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_username("username", "MQTT Username", mqtt_username, 32);
    WiFiManagerParameter custom_mqtt_password("password", "MQTT Password", mqtt_password, 32);
    WiFiManagerParameter custom_lamp1("lamp1", "Lampe 1 ID (HEX)", "", 32);
    WiFiManagerParameter custom_lamp2("lamp2", "Lampe 2 ID (HEX)", "", 32);
    WiFiManagerParameter custom_lamp3("lamp3", "Lampe 3 ID (HEX)", "", 32);

    wm.addParameter(&custom_mqtt_broker);
    wm.addParameter(&custom_mqtt_port);
    wm.addParameter(&custom_mqtt_username);
    wm.addParameter(&custom_mqtt_password);
    wm.addParameter(&custom_lamp1);
    wm.addParameter(&custom_lamp2);
    wm.addParameter(&custom_lamp3);

    wm.setConfigPortalTimeout(300); // 5 Minuten

    if (!wm.startConfigPortal("Lumos-Lampe"))
    {
        Serial.println("[WARN] Konnte kein WLAN verbinden, Neustart...");
        delay(3000);
        ESP.restart();
    }

    // Werte auslesen und speichern
    strcpy(mqtt_broker, custom_mqtt_broker.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_username, custom_mqtt_username.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());

    lamp1 = strtoul(custom_lamp1.getValue(), NULL, 16);
    lamp2 = strtoul(custom_lamp2.getValue(), NULL, 16);
    lamp3 = strtoul(custom_lamp3.getValue(), NULL, 16);

    partners[0] = lamp1;
    partners[1] = lamp2;
    partners[2] = lamp3;

    saveConfig();

    Serial.println("[INFO] WLAN und MQTT Parameter gespeichert");
}

// === MQTT ===
void callback(char *topic, byte *payload, unsigned int length)
{ // function die aufgerufen wird wenn auf den gefolgten MQTT kanal was gesendet wurde
    String message = String();
    for (int i = 0; i < length; i++)
    {
        message.concat((char)payload[i]);
    }
    // Serial.println(message);
    analogWrite(ledPin, message.toInt());
    // Serial.println("-----------------------");
    for (int i = 0; i < partners_count; i++)
    { // TODO: Was wenn ein lumos nicht mehr sendet und der letzte wert nicht 0 war?
        if (strcmp(String(partners[i], HEX).c_str(), topic) == 0)
        {
            int value = message.toInt();
            if (value > 255)
            {
                RGB_values[i] = 255;
            }
            else
            {
                RGB_values[i] = value;
            }
        }
    }
}


void reset_button_loop();

void build_MQTT_connection()
{
    client.setServer(mqtt_broker, atoi(mqtt_port));
    client.setCallback(callback);
    while (!client.connected())
    {
        reset_button_loop();
        String client_id = String(lumos_id, HEX);
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
        {
            Serial.println("Public EMQX MQTT broker connected");
        }
        else
        {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
}
void sub_to_lumos()
{
    for (int i = 0; i < partners_count; i++)
    {
        if (partners[i] != 0)
        {
            client.subscribe(String(partners[i], HEX).c_str());
            Serial.print("Subscrinbed to: ");
            Serial.println(String(partners[i], HEX).c_str());
        }
    }
}

// === Sound handling ===
int last_last_sound = 0;
int last_sound = 0;
int measureSound()
{
    int reading = analogRead(soundPin);
    if (reading < 0)
    {
        reading = -reading;
    }
    reading = (reading / 4);
    if (reading < 27)
    {
        reading = 0;
    }
    reading = (reading + 3 * last_sound) / 4;
    reading = (reading + last_last_sound) / 2;
    if (reading < 15)
    {
        reading = reading / 4;
    }
    last_last_sound = last_sound;
    last_sound = reading;
    return reading;
}

// === LED functions ===
void draw_led(int R, int G, int B)
{
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(R, G, B));
    }
    pixels.show();
}

void draw_led(int R, int G, int B, int Brightness)
{
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(
                                    static_cast<int>(static_cast<float>(R) * static_cast<float>(Brightness) / 100), // für Prozentrechnungen werden die Integer zu Floats gecastet und wieder zurück
                                    static_cast<int>(static_cast<float>(G) * static_cast<float>(Brightness) / 100),
                                    static_cast<int>(static_cast<float>(B) * static_cast<float>(Brightness) / 100)));
    }
    pixels.show();
}

void loading_animation(int R, int G, int B)
{
    while (true)
    {
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(i, pixels.Color(R, G, B));
            pixels.show();
            pixels.setPixelColor(i, pixels.Color(0, 0, 0));
            delay(50);
        }
    }
}
void loading_animation_MQTT(void *pvParameters)
{
    loading_animation(255, 0, 255);
}
void loading_animation_WIFI(void *pvParameters)
{
    loading_animation(0, 255, 255);
}

TaskHandle_t Wifi_loading;
TaskHandle_t MQTT_loading;

// xTaskCreatePinnedToCore(loading_animation_MQTT,"MQTT_loading",10000,NULL,1,&Wifi_loading,0);

// === Button action ===

void off_button_loop()
{ //
    if (digitalRead(turn_off_button) == LOW)
    {
        Serial.println("off Button pressed");
        for (int i = 0; i < 3; i++)
        {
            client.publish(String(lumos_id, HEX).c_str(), std::to_string(3).c_str());
            delay(50);
        }
        draw_led(0, 0, 0);
        delay(50);
        esp_deep_sleep_start(); // Geht in den DEEP_SLEEP wo nur noch der co-Prozessor aktiv ist.
    }
}

// === Reset Button ===
void reset_button_loop()
{

    if (digitalRead(reset_button) == LOW)
    {
        Serial.println("RESET Button pressed");
            startConfigPortal();
    }
}

// === Adaptive brightness ===
int loops_sins_brightness_measured = 0;
int led_brightness = 100;
void adapt_brightness()
{
    if (RGB_values[0] + RGB_values[1] + RGB_values[2] == 0 || loops_sins_brightness_measured > 500)
    {
        draw_led(0, 0, 0);
        int brightness = analogRead(lightSensPin);
        draw_led(RGB_values[0], RGB_values[1], RGB_values[2], led_brightness);
        if (brightness > 1500)
        {
            led_brightness = 100;
        }
        else if (brightness > 500)
        {
            led_brightness = 60;
        }
        else if (brightness > 200)
        {
            led_brightness = 10;
        }
        loops_sins_brightness_measured = 0;
    }
    loops_sins_brightness_measured++;
}

void setup()
{
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    pinMode(turn_off_button, INPUT_PULLUP);
    pinMode(reset_button, INPUT_PULLUP);

    // Lade gespeicherte MQTT-Konfig
    loadConfig();

    // WLAN verbinden oder Portal starten
    if (!wm.autoConnect("Lumos-Lampe"))
    {
        Serial.println("[WARN] Keine Verbindung -> Neustart");
        delay(3000);
        ESP.restart();
    }
    Serial.println("[INFO] Mit WLAN verbunden");

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, LOW);
    xTaskCreate(loading_animation_WIFI, "Wifi_loading", 10000, NULL, 1, &Wifi_loading); // Animation fürs laden die nebenläufig ist
    vTaskDelete(Wifi_loading);
    xTaskCreate(loading_animation_MQTT, "MQTT_loading", 10000, NULL, 1, &MQTT_loading); // Animation fürs laden die nebenläufig ist
    build_MQTT_connection();
    sub_to_lumos();
    vTaskDelete(MQTT_loading);
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED){
        Serial.println("WiFi reconnecting");
        xTaskCreate(loading_animation_WIFI, "Wifi_loading", 10000, NULL, 1, &Wifi_loading);
        WiFi.reconnect();
        vTaskDelete(Wifi_loading);
        Serial.println("WiFi reconnected");
    }
    else if (!client.loop()){
        Serial.println("MQTT Client reconnecting");
        xTaskCreate(loading_animation_MQTT, "MQTT_loading", 10000, NULL, 1, &MQTT_loading);
        build_MQTT_connection();
        sub_to_lumos();
        vTaskDelete(MQTT_loading);
        Serial.println("MQTT Client reconnected");
    }
    client.loop();
    client.publish(String(lumos_id, HEX).c_str(), std::to_string(measureSound()).c_str());
    adapt_brightness();
    draw_led(RGB_values[0], RGB_values[1], RGB_values[2], led_brightness);
    off_button_loop();
    reset_button_loop(); // Prüft ob der Reset Button gedrückt wurde
}