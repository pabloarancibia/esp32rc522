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

// NODO CONFIG
String topic_str = NODO_TOPIC;//topic para pub
String topic_error = TOPIC_ERROR;//topic para sub
String topic_confirm = TOPIC_CONFIRM;//topic para sub

// MQTT CONF
const char* mqttServer = MQTT_SERVER;
const int mqttPort = MQTT_PORT;
const char* mqttUser = MQTT_USER;
const char* mqttPassword = MQTT_PASSWORD;


// WIFI CONF
WiFiClient espClient;

PubSubClient client(espClient);


// LED function
void ledblink(int largo){
  digitalWrite(led, HIGH);
  delay(largo);
  digitalWrite(led, LOW);

}

// buzzer function
void bip(int largo, int toques){
  for (int t=0;t<toques;t++){
    digitalWrite(buzzer, HIGH);
    delay(largo);
    digitalWrite(buzzer, LOW);
    delay(200);//delay entre toques
  } 
    
}

// callback cuando lee tarjeta 
// envía el publish
void PublishMqtt() {
  
  
      if (rfid.PICC_ReadCardSerial())  // Leemos el contenido de la tarjeta
      {
        UIDCaracteres="";
        for (byte i = 0; i < rfid.uid.size; i++)
        {
           UIDCaracteres += rfid.uid.uidByte[i];
        }

        // Preparo el mensaje 
        String message = "{\"tarjeta\":\""+UIDCaracteres+"\",\"nodo\":\""+NODO_NOMBRE+"\",\"estado\":\""+NODO_ESTADO+"\"}";
      
        // String to char
        char message_buff[100];
        message.toCharArray(message_buff,message.length()+1);

        // topic_str a char
        char topic_buff[100];
        topic_str.toCharArray(topic_buff,topic_str.length()+1);

        // publish mensaje
        client.publish(topic_buff,message_buff);
        
        // led y beep
        Serial.println(message);
        digitalWrite(led, HIGH);
        bip(500, 1);
        digitalWrite(led, LOW);
        delay(800);//delay para proxima lectura

        
      }else{
        Serial.println("Error en lectura");
        bip(150,3);//bip error
      }
      UIDCaracteres=""; 
}
  

// realiza las suscripción a los topic
void SuscribeMqtt()
{

    // topic error
    char topic_error_buff[100];
    topic_error.toCharArray(topic_error_buff,topic_error.length()+1);
    client.subscribe(topic_error_buff);
    
    Serial.print("suscripto a topic: ");
    Serial.print(topic_error_buff);
    Serial.println();

    // topic confirm
    char topic_confirm_buff[100];
    topic_confirm.toCharArray(topic_confirm_buff,topic_confirm.length()+1);
    client.subscribe(topic_confirm_buff);
    
    Serial.print("suscripto a topic: ");
    Serial.print(topic_confirm_buff);
    Serial.println();
}

// callback a ejecutar cuando se recibe un mensaje
void OnMqttReceived(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Received on ");
    Serial.print(topic);
    Serial.print(": ");
    String content = "";
    for (size_t i = 0; i < length; i++)
    {
        content.concat((char)payload[i]);
    }
    //imprimo en consola payload
    Serial.print(content);
    Serial.println();

    // imprimo en consola topic recibido y topic error configurado en este nodo
    Serial.println();
    Serial.print("Topic recibido ");
    Serial.println(topic);
    Serial.print("Topic error configurado en este nodo ");
    Serial.println(topic_error);
     Serial.print("Topic confirm configurado en este nodo ");
    Serial.println(topic_confirm);
    Serial.println();


    // paso string a char
    char topic_error_buff[100];
    topic_error.toCharArray(topic_error_buff,topic_error.length()+1);
    
    char topic_confirm_buff[100];
    topic_confirm.toCharArray(topic_confirm_buff,topic_confirm.length()+1);

    // comparo si topic recibido es igual a topic configurado en este nodo
    int result = strcmp(topic,topic_error_buff);
    if (result==0)
    {
      // topic error recibido corresponde a este nodo
      Serial.println("ERROR");
      Serial.println();

      // toca 6 veces 
      digitalWrite(led, HIGH);
      bip(150,3);//bip error
      delay(200);
      bip(150,3);//bip error
      digitalWrite(led, LOW);
    }

    int result_confirm = strcmp(topic,topic_confirm_buff);
    if (result_confirm==0)
    {
      // topic confirm recibido corresponde a este nodo
      Serial.println("CONFIRM OK");
      Serial.println();
      // Toca 2 veces
      digitalWrite(led, HIGH);
      bip(200,2);//bip confirm
      digitalWrite(led, LOW);
    }
}

// inicia la comunicacion MQTT
// inicia establece el servidor y el callback al recibir un mensaje
void InitMqtt()
{
    client.setServer(mqttServer, mqttPort);
    client.setCallback(OnMqttReceived);
}

void mqttConnect(){
  

  client.setServer(mqttServer, mqttPort);
  
  while (!client.connected()) {
      Serial.println("Connecting to MQTT...Server: ");
      Serial.print(mqttServer);
      Serial.print(" Port: ");
      Serial.print(mqttPort);
      Serial.print(" User: ");
      Serial.print(mqttUser);
      Serial.print(" Nombre: ");
      Serial.print(NODO_NOMBRE);
      Serial.println();
  
      // if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
      if (client.connect(NODO_NOMBRE,mqttUser, mqttPassword)) {

        

        Serial.print("CONECTADO a mqtt en: Server: ");
        Serial.print(MQTT_SERVER);
        Serial.print(" Port:");
        Serial.print(MQTT_PORT);
        Serial.print(" User:");
        Serial.print(mqttUser);
        Serial.print(" Nombre:");
        Serial.print(NODO_NOMBRE);
        Serial.println();

        SuscribeMqtt();

        
      } else {
  
        Serial.print("failed with state ");
        Serial.print(client.state());
        Serial.println(" try again in 2 seconds");
        digitalWrite(led, HIGH);
        delay(2000);
        digitalWrite(led, LOW);
  
      }
  }

    
  client.publish("esp/test", "Hello from ESP32+RFIDrc522");
}

void init_setup(){
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  SPI.begin();
  rfid.PCD_Init();//Inicilializar lector
  Serial.begin(115200); // Velocidad del terminal serial
}

//WIFI SETUP
void wifi_setup() {
  

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

  
  }

void RfidRcv(){
  if (rfid.PICC_IsNewCardPresent())  // Hay una nueva tarjeta presente
    {
      PublishMqtt();
    }
    client.loop();

}




// gestiona la comunicación MQTT
// comprueba que el cliente está conectado
// no -> intenta reconectar
// si -> llama al MQTT loop
void HandleMqtt()
{
    if (!client.connected())
    {
        //llamar a coneccion de mqtt
        mqttConnect();
    }
    client.loop();
}

// únicamente inicia seria, ethernet y MQTT
void setup()
{
    init_setup();
    wifi_setup();
    delay(1500);
    InitMqtt();
}

// únicamente llama a HandleMqtt
void loop()
{
    HandleMqtt();
    RfidRcv(); //escucha rfid
}