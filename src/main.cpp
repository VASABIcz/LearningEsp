#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_BMP280.h>
#include "Wire.h"

struct Data {
    float temperature;
    float altitude;
    float pressure;
    bool success;
};

Adafruit_BMP280 bmp;
WiFiServer server(6000);
Data data{0, 0, 0, false};

void setupI2c() {
    Wire.begin();
    if (!bmp.begin(0x76)) {
        Serial.println("failed to setup bmp halting!!");
        while (1) delay(100);
    }
    bmp.setSampling(Adafruit_BMP280::MODE_FORCED,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
    Serial.println("bmp setup and configured");
}

void setupNetwork() {
    // WiFi.softAP("eps-test-lol", "gigasecurepassword");
    // WiFi.mode(WIFI_STA);
    WiFi.begin("TP-LINK_41D52A");
    delay(50);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("connected ");
    Serial.println(WiFi.localIP());
    server.begin();
    server.hasClient();
}

void serverLoop() {
    WiFiClient client = server.available();
    while (client) {
        Serial.print("serving: ");
        client.remoteIP().printTo(Serial);
        Serial.println();
        if (client.connected()) {
            Serial.printf("writing data %d", sizeof(data));
            Serial.println();
            client.write(reinterpret_cast<const char *>(&data), sizeof(data));
            Serial.printf("wrote data %d", sizeof(data));
            Serial.println();
        }
        else {
            Serial.print("disconnecting: ");
            client.remoteIP().printTo(Serial);
            Serial.println();
            client.stop();
        }
        if (client.next == nullptr) break;
        client = *client.next;
    }
}

void setup() {
    Serial.begin(115200);
    delay(5000);
    Serial.println("start");

    setupI2c();
    setupNetwork();
}

Data measure() {
    Data d{};
    if (bmp.takeForcedMeasurement()) {
        d.altitude = bmp.readAltitude(1013.25);
        d.pressure = bmp.readAltitude();
        d.temperature = bmp.readTemperature();
        d.success = true;
    }
    else {
        Serial.println("failed to take measurements");
        d.success = false;
    }
    return d;
}


void loop() {
    data = measure();
    serverLoop();
}
