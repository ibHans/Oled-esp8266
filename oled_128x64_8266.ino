
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h> 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET D4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char *ssid = "wifissid"; // Change this to your WiFi SSID
const char *password = "wifipass"; // Change this to your WiFi password
const String ducoUser = "username"; // Change this to your Duino-Coin username
const char* MINER_KEY = "None"; // Change the part in brackets to your mining key (if you enabled it in the wallet)
const String ducoReportJsonUrl = "https://server.duinocoin.com/v2/users/" + ducoUser + "?limit=1";
const int run_in_ms = 2000;
float lastAverageHash = 0.0;
float lastTotalHash = 0.0;

void setup() {
    Serial.begin(115200);
    setupWifi();
    initDisplayOled();
    }

void setupWifi() {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");}
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    }
    String httpGetString(String URL) {
    String payload = "";
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    if (http.begin(client, URL)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            payload = http.getString();
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());}
        http.end();}
    return payload;
    }

void initDisplayOled() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
        }
    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.clearDisplay();
    delay(500); // Pause for 2 seconds
    // Clear the buffer
    display.clearDisplay();
    }
    boolean runEvery(unsigned long interval) {
    static unsigned long previousMillis = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        return true;}
    return false;
    }

void loop() {
    if (runEvery(run_in_ms)) {

        float totalHashrate = 0.0;
        float avgHashrate = 0.0;
        int total_miner = 0;
        String input = httpGetString(ducoReportJsonUrl);
        DynamicJsonDocument doc (8000);
        DeserializationError error = deserializeJson(doc, input);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;}
        JsonObject result = doc["result"];
        JsonObject result_balance = result["balance"];
        double result_balance_balance = result_balance["balance"];
        const char *result_balance_created = result_balance["created"];
        const char *result_balance_username = result_balance["username"];
        const char *result_balance_verified = result_balance["verified"];
        
        
        for (JsonObject result_miner : result["miners"].as<JsonArray>()) {
            float result_miner_hashrate = result_miner["hashrate"];
            totalHashrate = totalHashrate + result_miner_hashrate;
            total_miner++;}

        avgHashrate = totalHashrate / long(total_miner);
        long run_span = run_in_ms / 1000;

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println(String(result_balance_balance)+ " Duco");
        display.println();
        display.println(String(result_balance_username));
        display.println(String(total_miner)+ " worker");
        display.println(String(totalHashrate/1000)+ " Kh/s");
        display.display();
    }
}
