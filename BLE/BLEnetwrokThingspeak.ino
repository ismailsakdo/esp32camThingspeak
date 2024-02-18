#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ThingSpeak.h>
#include <WiFi.h>

#define relay 2

String knownBLEAddresses[] = {"XXXXX", "XXXXX"};
int RSSI_THRESHOLD = -100;
bool device_found;
int scanTime = 1; // In seconds
BLEScan* pBLEScan;
unsigned long previousMillis = 0;
const long interval = 60000; // Interval for sending data to ThingSpeak (milliseconds)

const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
const char* server = "api.thingspeak.com";
const char* apiKey = "YourAPIKey";

WiFiClient client; // Declaration of WiFiClient object

void sendToThingSpeak(int rssi, int doorStatus) {
  if (client.connect(server, 80)) {
    String postStr = "api_key=";
    postStr += apiKey;
    postStr += "&field1=";
    postStr += String(rssi);
    postStr += "&field2=";
    postStr += String(doorStatus);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + String(apiKey) + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.println("Data sent to ThingSpeak.");
  } else {
    Serial.println("Failed to connect to ThingSpeak server.");
  }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      for (int i = 0; i < (sizeof(knownBLEAddresses) / sizeof(knownBLEAddresses[0])); i++) {
        if (strcmp(advertisedDevice.getAddress().toString().c_str(), knownBLEAddresses[i].c_str()) == 0) {
          device_found = true;
          break;
        } else {
          device_found = false;
        }
      }
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    }
};

void setup() {
  Serial.begin(115200); // Enable UART on ESP32
  Serial.println("Scanning..."); // Print Scanning
  pinMode(relay, OUTPUT); // Make relay pin as output
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); // Create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); // Init Callback Function
  pBLEScan->setActiveScan(true); // Active scan uses more power, but gets results faster
  pBLEScan->setInterval(100); // Set scan interval
  pBLEScan->setWindow(99);  // Less or equal setInterval value

  WiFi.begin(ssid, password);
  ThingSpeak.begin(client);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    BLEScanResults* foundDevices = pBLEScan->start(scanTime, false);
    for (int i = 0; i < foundDevices->getCount(); i++) {
      BLEAdvertisedDevice device = foundDevices->getDevice(i);
      int rssi = device.getRSSI();
      Serial.print("RSSI: ");
      Serial.println(rssi);
      if (rssi > RSSI_THRESHOLD && device_found == true) {
        Serial.println("Door Open");
        digitalWrite(relay, HIGH);
        sendToThingSpeak(rssi, 1); // Sending door status as 1 (open)
        delay(5000);
        Serial.println("Door Close");
        digitalWrite(relay, LOW);
        sendToThingSpeak(rssi, 0); // Sending door status as 0 (closed)
      }
    }
    pBLEScan->clearResults(); // Delete results from BLEScan buffer to release memory
    previousMillis = currentMillis;
  }
}
