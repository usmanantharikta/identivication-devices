/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include "MFRC522.h"
#define RST_PIN D1 // RST-PIN for RC522 - RFID - SPI - Modul GPIO15
#define SS_PIN  D2  // SDA-PIN for RC522 - RFID - SPI - Modul GPIO2
#define LED D8
#define buzzer D3

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

const char* ssid     = "hayah";
const char* password = "usman@1234";

const char* host = "192.168.43.63"; // connect to host on web

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(LED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //RFID
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
}

int value = 0;

void loop() {
   digitalWrite(LED_BUILTIN, HIGH);
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  // Show some details of the PICC (that is: the tag/card)
  //MSerial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  // We now create a URI for the request
  String url = "/sls/admin/visitor";
  String uid="";
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : "");
    Serial.print(buffer[i], DEC);
    uid += buffer[i];
  }

   digitalWrite(LED, HIGH);
//  url+="_rack-01";
//  url+="_wemos-01";
String data="rfid=";
data+=uid;
data+="&module=module1";
data+="&wemos=wemos1";

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n");

//client.println("POST "+url+" HTTP/1.1");
//client.println("Host: "+ host);
client.println("Cache-Control: no-cache");
client.println("Content-Type: application/x-www-form-urlencoded");
client.print("Content-Length: ");
client.println(data.length());
client.println();
client.println(data);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
//    delay(250);
    String line = client.readStringUntil('\r');
    if(line=="HTTP/1.1 200 OK")
    {
      digitalWrite(LED_BUILTIN, LOW);
      tone(buzzer,110,100);
      delay(250);
      Serial.print("success");
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LED, LOW);
      noTone(buzzer);
    }
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
  digitalWrite(LED, LOW);
}
