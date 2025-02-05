#include <WiFi.h>
#include <HTTPClient.h>
#include <MFRC522.h>
#define RST_PIN 22
#define SS_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "Hashira09";
const char* password = "12345678";

const char* serverURL = "https://e1d4-112-204-168-184.ngrok-free.app/rfid_login";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  SPI.begin();        
  rfid.PCD_Init();    
  Serial.println("Place your card...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  Serial.println("UID: " + uid);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String requestBody = "{\"uid\": \"" + uid + "\"}";
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response: " + response);
    } else {
      Serial.println("Error: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("Wi-Fi Disconnected");
  }

  delay(2000);
}
