#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 22
#define SS_PIN 5
#define RELAY_PIN 27

const char* ssid = "Hashira09";         
const char* password = "12345678"; 
const char* serverUrl = "https://<YOUR_SERVER_IP>:3000/api/v1/rfid_data";

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); 

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
}

void loop() {
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

    String payload = String("{\"uid\":\"") + uid + String("\"}");
    int httpResponseCode = http.POST(payload);
    Serial.println("Payload sent: " + payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Server Response: " + response);

      
      if (response.indexOf("\"unlock\":true") != -1) {
        Serial.println("Access granted! Unlocking solenoid...");
        digitalWrite(RELAY_PIN, HIGH); 
        delay(5000);                  
        digitalWrite(RELAY_PIN, LOW); 
      } else {
        Serial.println("Access denied!");
      }
    } else {
      Serial.println("Error sending POST request");
    }
    http.end();
  }

  
  rfid.PICC_HaltA();

  delay(2000); 
}
