#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#include <WiFi.h>
#include <PubSubClient.h>

//RFID CONF
const int pinRST = 15;  // Pin RST del módulo RC522
const int pinSDA = 5; // pin SDA del módulo RC522
const int buzzer = 27;
const int led = 2;//Led onboard del esp32

MFRC522 rfid(pinSDA, pinRST);
String UIDCaracteres;//UID de tarjeta rfid


WiFiClient espClient;
PubSubClient client(espClient);

//RFID WIFI MQTT SETUP
void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  SPI.begin();
  rfid.PCD_Init();//Inicilializar lector
  Serial.begin(115200); // Velocidad del terminal serial

  //wifi
  const char* ssid = "ZulemayPablo";
  const char* password = "P4bl0yZul3";

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {   
      Serial.println("Connecting to WiFi..");
      Serial.println(WiFi.status());
      delay(1000);
  }
 
  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());

  //mqtt
  const char* mqttServer = "192.168.2.120";
  const int mqttPort = 1883;
  const char* mqttUser = "admin";
  const char* mqttPassword = "4dm1n/t3st";

  client.setServer(mqttServer, mqttPort);
  
  while (!client.connected()) {
      Serial.println("Connecting to MQTT...");
  
      if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
  
        Serial.println("connected");  
  
      } else {
  
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
  
      }
  }

    
  client.publish("esp/test", "Hello from ESP32");
  }

//Sonido del buzzer
void bip(int largo, int toques){
  for (int t=0;t<toques;t++){
    digitalWrite(buzzer, HIGH);
    delay(largo);
    digitalWrite(buzzer, LOW);
    delay(200);//delay entre toques
  } 
    
}

void loop() {
  if (rfid.PICC_IsNewCardPresent())  // Hay una nueva tarjeta presente
    {
      if (rfid.PICC_ReadCardSerial())  // Leemos el contenido de la tarjeta
      {
        Serial.println("UID de la tarjeta: ");
        for (byte i = 0; i < rfid.uid.size; i++)
        {
           UIDCaracteres += rfid.uid.uidByte[i];
        }
        client.publish("esp/test", "Tarjeta Leída" );
        Serial.print(UIDCaracteres);
        Serial.println();
        Serial.println("Tarjeta Correcta");
        digitalWrite(led, HIGH);
        bip(500, 1);
        digitalWrite(led, LOW);
        delay(800);//delay para proxima lectura
      }else{
        Serial.println("Error en lectura");
        bip(150,3);//bip error
      } 
    }
  UIDCaracteres="";  
}
