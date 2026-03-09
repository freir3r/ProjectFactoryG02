#include "DFRobotDFPlayerMini.h"

// Usamos a Serial2 do ESP32 (Pinos 16 e 17)
HardwareSerial somSerial(2); 
DFRobotDFPlayerMini myDFPlayer;

void setup() {
  Serial.begin(115200);
  somSerial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("A iniciar DFPlayer...");
  delay(3000);

  if (!myDFPlayer.begin(somSerial)) {
    Serial.println("Erro: Não foi possível comunicar com o modulo.");
    Serial.println("1. Verifica se o cartão SD está em FAT32.");
    Serial.println("2. Verifica se a pasta se chama 'mp3' e o ficheiro '0001.mp3'.");
    while(true); 
  }

  Serial.println("DFPlayer OK! A tocar música...");
  
  myDFPlayer.volume(30); // Volume (0 a 30)
  myDFPlayer.play(1);    // Toca o ficheiro /mp3/0001.mp3
}

void loop() {
  // Deixamos o loop vazio para o teste inicial
}