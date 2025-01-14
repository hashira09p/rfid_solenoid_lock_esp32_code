#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#define RST_PIN 22
#define SS_PIN 5
#define RELAY_PIN 27

String room_status = "Lock";

const char* ssid = "Hashira09";
const char* password = "12345678";
const char* serverUrl = "https://95bf-112-204-162-196.ngrok-free.app/rfids";

MFRC522 rfid(SS_PIN, RST_PIN);

// Initialize LCD
hd44780_I2Cexp lcd;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  
  rfid.PCD_Init();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Wire.begin(21, 17); // SDA = GPIO 21, SCL = GPIO 17
  lcd.begin(16, 2);   // 16 columns, 2 rows
  lcd.clear();
  lcd.print("TUP System");
  lcd.setCursor(0, 1);
  lcd.print("Initialized!");
  delay(2000);
  lcd.clear();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("internet...");
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.print("Wi-Fi Connected");
  Serial.println("\nConnected to Wi-Fi!");
  delay(2000);
  lcd.clear();
  lcd.print("Please scan");
  lcd.setCursor(0, 1);
  lcd.print("your RFID Card!");
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

  lcd.clear();
  lcd.print("Getting Card");
  lcd.setCursor(0, 1); 
  lcd.print("Information");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "6dbe948bb56f1d6827fbbd8321c7ad14");

    String payload = String("{\"uid\":\"") + uid + String("\",\"room_number\":\"") + 201 +
                     String("\",\"room_status\":\"") + room_status + String("\"}");

    int httpResponseCode = http.POST(payload);
    Serial.println("Payload sent: " + payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Server Response: " + response);

      if (response.indexOf("\"unlock\":true") != -1 && digitalRead(RELAY_PIN) == LOW) {
        room_status = "Unlock";
        delay(1000);
        int serverResponse = response.indexOf("\"user\":");
        String username = response.substring(serverResponse + 8 , serverResponse + 14);
        
        lcd.clear();
        lcd.print("Access Granted!");
        delay(1000);
        lcd.clear();
        digitalWrite(RELAY_PIN, HIGH);

        lcd.print("Welcome Sir");
        lcd.setCursor(0, 1);
        lcd.print(username);
        Serial.println(username);
        delay(2000);
        lcd.clear();
        
        Serial.println("Access granted! Unlocking solenoid...");

        lcd.print("Room 201");
        lcd.setCursor(0, 1);
        lcd.print("Unlocked!");
      } else if (response.indexOf("\"lock\":true") != -1 && digitalRead(RELAY_PIN) == HIGH) {
        room_status = "Lock";
        delay(1000);
        int serverResponse = response.indexOf("\"user\":");
        String username = response.substring(serverResponse + 8, serverResponse + 14);

        lcd.clear();
        lcd.print("Access Granted!");
        delay(1000);
        lcd.clear();
        digitalWrite(RELAY_PIN, LOW);

        lcd.print("Goodbye Sir");
        lcd.setCursor(0, 1);
        lcd.print(username);
        Serial.println(username);
        delay(2000);
        lcd.clear();
        
        Serial.println("Access granted! Locking solenoid...");

        lcd.print("Room 201");
        lcd.setCursor(0, 1);
        lcd.print("Locked!");
      } else {
        Serial.println("Access denied!");
        lcd.clear();
        lcd.print("Access Denied");
      }
    } else {
      Serial.println("Error sending POST request");
      lcd.clear();
      lcd.print("POST Request Err");
    }
    http.end();
  }

  rfid.PICC_HaltA();
  delay(2000);
}
