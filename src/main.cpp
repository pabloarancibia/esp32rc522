#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

const int pinRST = 15;  // Pin RST del módulo RC522
const int pinSDA = 5; // pin SDA del módulo RC522
const int buzzer = 27;
const int led = 2;//Led onboard del esp32

MFRC522 rfid(pinSDA, pinRST);
String UIDCaracteres;//UID de tarjeta rfid

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  SPI.begin();
  rfid.PCD_Init();//Inicilializar lector
  Serial.begin(115200); // Velocidad del terminal serial
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
