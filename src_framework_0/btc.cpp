#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
void appBitcoin() {

    // WiFi settings
    const char* ssid     = "ssid";
    const char* password = "passcode";

    // API server
    const char* host = "api.coindesk.com";

    int16_t x, y;
    ttgo->tft->fillScreen(TFT_PURPLE);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {}

    while (ttgo->getTouch(x, y)) {}

    WiFiClient client;
    const int httpPort = 80;
    client.connect(host, httpPort);

    // We now create a URI for the request
    String url = "/v1/bpi/currentprice.json";

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(100);

    // Read all the lines of the reply from server and print them to Serial
    String answer;
    while (!client.available()) {}
    while (client.available()) {
        String line = client.readStringUntil('\r');
        answer += line;
    }
client.stop();

// Convert to JSON
String jsonAnswer;
int jsonIndex;

for (int i = 0; i < answer.length(); i++) {
if (answer[i] == '{') {
jsonIndex = i;
break;
}
}

// Get JSON data
jsonAnswer = answer.substring(jsonIndex);
jsonAnswer.trim();

// Get rate as float
int rateIndex = jsonAnswer.indexOf("rate_float");
String priceString = jsonAnswer.substring(rateIndex + 12, rateIndex + 19);
priceString.trim();
float price = priceString.toFloat();

ttgo->tft->setTextSize(2);
ttgo->tft->setTextColor(TFT_GREEN);
ttgo->tft->setCursor(40, 90);

ttgo->tft->println("Bitcoin Price");
ttgo->tft->setCursor(70, 130);
ttgo->tft->println(priceString);

client.stop();
WiFi.disconnect(true);
WiFi.mode(WIFI_OFF);

while (!ttgo->getTouch(x, y)) {} // wait until you touching
while (ttgo->getTouch(x, y)) {}
ttgo->tft->fillScreen(TFT_BLACK);
}