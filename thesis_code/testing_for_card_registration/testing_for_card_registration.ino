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
#define BUTTON_PIN 16

String room_status = "Lock";

const char* ssid = "Hashira09";
const char* password = "12345678";
const char* accessUrl = "https://b7e3-112-204-162-196.ngrok-free.app/rfids";
const char* registerUrl = "https://b7e3-112-204-162-196.ngrok-free.app/cards";

MFRC522 rfid(SS_PIN, RST_PIN);

hd44780_I2Cexp lcd;
HTTPClient accesshttp;
HTTPClient registerhttp;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUTTON_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Wire.begin(21, 17);  // SDA = GPIO 21, SCL = GPIO 17
  lcd.begin(16, 2);    // 16 columns, 2 rows
  lcd.clear();
  lcd.print("TUP System");
  lcd.setCursor(0, 1);
  lcd.print("Initialized!");
  delay(2000);
  lcd.clear();

  wifiConnect(ssid, password);

  lcd.clear();
  lcd.print("Please scan");
  lcd.setCursor(0, 1);
  lcd.print("your RFID Card!");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  String uid = "";
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == 1) {
    Serial.println("Button State: HIGH");

    lcd.clear();
    lcd.print("REGISTRATION");
    lcd.setCursor(0, 1);
    lcd.print("MODE");
    delay(1000);

    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    Serial.println("Card UID: " + uid);
    registerMode(uid);
  } else {

    Serial.println("Button State: LOW");

    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    Serial.println("Card UID: " + uid);

    lcd.clear();
    lcd.print("ACCESS MODE");
    accessMode(uid);
  }
}

void wifiConnect(String wifiName, String wifiPassword) {
  WiFi.begin(wifiName, wifiPassword);
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
}

void accessMode(String cardUid) {
  lcd.clear();
  lcd.print("Getting Card");
  lcd.setCursor(0, 1);
  lcd.print("Information");
  delay(1000);

  if (WiFi.status() == WL_CONNECTED) {
    accesshttp.begin(accessUrl);
    accesshttp.addHeader("Content-Type", "application/json");
    accesshttp.addHeader("Authorization", "6dbe948bb56f1d6827fbbd8321c7ad14");

    String payload = String("{\"uid\":\"") + cardUid + String("\",\"room_number\":\"") + 201 + String("\",\"room_status\":\"") + room_status + String("\"}");

    int httpResponseCode = accesshttp.POST(payload);
    Serial.println("Payload sent: " + payload);

    if (httpResponseCode > 0) {
      String response = accesshttp.getString();
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Server Response: " + response);

      if (response.indexOf("\"unlock\":true") != -1 && digitalRead(RELAY_PIN) == LOW) {
        room_status = "Unlock";
        delay(1000);
        int serverResponse = response.indexOf("\"user\":");
        String username = response.substring(serverResponse + 8, serverResponse + 14);

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
    accesshttp.end();
  }

  rfid.PICC_HaltA();
  delay(2000);
}

void registerMode(String cardUid) {
  lcd.clear();
  lcd.print("Getting Card");
  lcd.setCursor(0, 1);
  lcd.print("Information");
  delay(1000);

  if (WiFi.status() == WL_CONNECTED) {
    registerhttp.begin(registerUrl);
    registerhttp.addHeader("Content-Type", "application/json");
    registerhttp.addHeader("Authorization", "6dbe948bb56f1d6827fbbd8321c7ad14");

    String payload = String("{\"uid\":\"") + cardUid + String("\"}");

    int httpResponseCode = registerhttp.POST(payload);
    Serial.println("Payload sent: " + payload);

    if (httpResponseCode > 0) {
      String response = registerhttp.getString();
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Server Response: " + response);

      if (response.indexOf("\"success\":true") != -1) {
        delay(1000);
        lcd.clear();
        lcd.print("Success");
        delay(1000);
      } else if (response.indexOf("\"success\":false") != -1) {
        lcd.clear();
        lcd.print("The card is");
        lcd.setCursor(0, 1);
        lcd.print("already in use");
        delay(1000);
      }
    } else {
      Serial.println("Error sending POST request");
      lcd.clear();
      lcd.print("POST Request Err");
    }
    registerhttp.end();
  }

  rfid.PICC_HaltA();
  delay(2000);
}