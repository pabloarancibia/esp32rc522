#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <env.h>

//RFID CONF
const int pinRST = 15;  // Pin RST del módulo RC522
const int pinSDA = 5; // pin SDA del módulo RC522
const int buzzer = 27;
const int led = 2;//Led onboard del esp32

MFRC522 rfid(pinSDA, pinRST);
String UIDCaracteres;//UID de tarjeta rfid
String topic_str = NODO_TOPIC;//topic


WiFiClient espClient;
PubSubClient client(espClient);

void ledblink(int largo){
  digitalWrite(led, HIGH);
  delay(largo);
  digitalWrite(led, LOW);

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
void mqttConnect(){
  //mqtt
  const char* mqttServer = MQTT_SERVER;
  const int mqttPort = MQTT_PORT;
  const char* mqttUser = MQTT_USER;
  const char* mqttPassword = MQTT_PASSWORD;

  client.setServer(mqttServer, mqttPort);
  
  while (!client.connected()) {
      Serial.println("Connecting to MQTT...");
  
      // if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
      if (client.connect(NODO_NOMBRE)) {

        Serial.print("conectado a mqtt en: ");
        Serial.print(MQTT_SERVER);
        Serial.print(":");
        Serial.print(MQTT_PORT);
        Serial.println();

        
      } else {
  
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
  
      }
  }

    
  client.publish("esp/test", "Hello from ESP32+RFIDrc522");
}

//RFID WIFI MQTT SETUP
void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  SPI.begin();
  rfid.PCD_Init();//Inicilializar lector
  Serial.begin(115200); // Velocidad del terminal serial

  //wifi
  const char* ssid = WIFI_LOCAL_SSID;
  //const char* password = WIFI_LOCAL_PASS;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid);

  

  while (WiFi.status() != WL_CONNECTED) {   
      Serial.println("Connecting to WiFi..");
      Serial.println(WiFi.status());
      Serial.println(ssid);
      Serial.println();
      Serial.print("ESP Board MAC Address:  ");
      Serial.println(WiFi.macAddress());
      ledblink(300);
      delay(1000);
  }
 
  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  ledblink(5000);
  bip(200,2);//conexion wifi exitosa

  //llamar a coneccion de mqtt
  mqttConnect();
  }




void loop() {
  while (!client.connected()) {
    Serial.print(client.state());
    mqttConnect();
  }
  
  if (rfid.PICC_IsNewCardPresent())  // Hay una nueva tarjeta presente
    {
      if (rfid.PICC_ReadCardSerial())  // Leemos el contenido de la tarjeta
      {
        // Serial.println("UID de la tarjeta: ");
        for (byte i = 0; i < rfid.uid.size; i++)
        {
           UIDCaracteres += rfid.uid.uidByte[i];
        }

        String message = "{tajeta:"+UIDCaracteres+",nodo:"+NODO_NOMBRE+",estado:"+NODO_ESTADO+"}";
        
        char message_buff[100];
        message.toCharArray(message_buff,message.length()+1);

        char topic_buff[100];
        topic_str.toCharArray(topic_buff,topic_str.length()+1);


        client.publish(topic_buff,message_buff,2);
        Serial.println(message);
        // Serial.println();
        // Serial.println("Tarjeta Correcta");
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
