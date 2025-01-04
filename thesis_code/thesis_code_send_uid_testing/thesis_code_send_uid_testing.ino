#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 22
#define SS_PIN 5

const char* ssid = "Hashira09";         
const char* password = "12345678"; 
const char* serverUrl = "https://9a57-112-204-168-184.ngrok-free.app/rfid_data";

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
}

void loop(){
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  Serial.println("Card UID: " + uid);


  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "6dbe948bb56f1d6827fbbd8321c7ad14");

    String payload = String("{\"uid\":\"") + uid + String("\"}");

    int httpResponseCode = http.POST(payload);
    Serial.println("Payload sent: " + payload);

    if (httpResponseCode > 0) {
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Server Response: " + http.getString());
    } else {
      Serial.println("Error sending POST request");
    }
    http.end();
  }

  rfid.PICC_HaltA();

  delay(2000);
}
