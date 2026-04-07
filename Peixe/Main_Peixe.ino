#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include "DFRobotDFPlayerMini.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int pinoServoPeixe = 13;   
const int pinoServoTampas = 14;  
const int pinosLDR[] = {34, 35, 32, 33, 25}; 

int etapaAtual = 0; 
int limiteEscuro = 2000;         
bool sistemaFinalizado = false;
bool aguardandoServo = false;     
bool movimentoConcluido = false;
bool ecrãDesligado = false;
unsigned long tempoVitoria = 0;
unsigned long tempoEsperaPosAudio = 0; 
float pescadorX = 30; 

//Servos
Servo servoPeixe;
Servo servoTampas;
unsigned long tempoServo = 0;
int estadoServo = 0; 
int contadorAbanar = 0;

//animação
float t = 0;
struct Point { float x, y; };
const int segments = 15;
Point rod[segments];

//SISTEMA DE FOGO DE ARTIFÍCIO
#define MAX_PARTICULAS 20
struct Particula { float x, y, dx, dy; int vida; };
Particula fogos[MAX_PARTICULAS];
bool fogoAtivo = false;
unsigned long tempoProximoFogo = 0;

HardwareSerial somSerial(2); 
DFRobotDFPlayerMini myDFPlayer;

//desenhos
void criarExplosao(int centerX, int centerY) {
  for (int i = 0; i < MAX_PARTICULAS; i++) {
    fogos[i].x = centerX; fogos[i].y = centerY;
    float angulo = random(0, 360) * 3.14159 / 180.0;
    float velocidade = random(10, 30) / 10.0; 
    fogos[i].dx = cos(angulo) * velocidade;
    fogos[i].dy = sin(angulo) * velocidade;
    fogos[i].vida = random(15, 30);
  }
  fogoAtivo = true;
}

void desenharFogoDeArtificio() {
  if (!fogoAtivo) return;
  bool algumaViva = false;
  for (int i = 0; i < MAX_PARTICULAS; i++) {
    if (fogos[i].vida > 0) {
      algumaViva = true;
      fogos[i].x += fogos[i].dx; fogos[i].y += fogos[i].dy;
      fogos[i].dy += 0.05; fogos[i].vida--;
      if (fogos[i].x >= 0 && fogos[i].x < SCREEN_WIDTH && fogos[i].y >= 0 && fogos[i].y < SCREEN_HEIGHT) {
        display.drawPixel((int)fogos[i].x, (int)fogos[i].y, WHITE);
      }
    }
  }
  if (!algumaViva) fogoAtivo = false;
}

void desenharAnimacaoPescador() {
  if (ecrãDesligado) return;
  display.clearDisplay();
  unsigned long agora = millis();
  
  if (sistemaFinalizado) {
    if (agora - tempoVitoria > 3000) {
      if (pescadorX > -50) pescadorX -= 0.8; 
      else if (!fogoAtivo && agora - tempoVitoria > 10000) {
        display.clearDisplay(); display.display();
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        ecrãDesligado = true;
        return;
      }
    }
    if (agora > tempoProximoFogo && !fogoAtivo && pescadorX > -20) {
      criarExplosao(random(60, 110), random(10, 30));
      tempoProximoFogo = agora + random(2000, 4000);
    }
    desenharFogoDeArtificio();
  }

  for (int x = 50; x < 128; x += 4) {
    display.drawRect(x, 50 + sin(x * 0.1 + t) * 2, 2, 2, WHITE);
  }
  display.fillRect(0, 40, 52, 3, WHITE); 
  display.fillRect(10, 43, 3, 20, WHITE); 
  display.fillRect(45, 43, 3, 20, WHITE);

  int px = (int)pescadorX; int py = 30;
  if (sistemaFinalizado && px > -45) py += abs((int)(sin(t * 6) * 2)); 

  if (px > -15 && px < 128) {
    display.fillRect(px, py, 6, 11, WHITE);          
    display.fillRect(px + 2, py + 4, 8, 2, WHITE);   
    display.fillRect(px - 1, py - 6, 8, 6, WHITE);   
    display.drawFastHLine(px - 3, py - 6, 12, WHITE); 
    display.fillRect(px - 1, py - 8, 8, 3, WHITE);   

    if (sistemaFinalizado) {
      if (agora - tempoVitoria < 1000) {
        display.drawLine(px + 6, py + 4, px + 20, py - 10, WHITE);
        display.fillTriangle(px+18, py+5, px+22, py+5, px+20, py+12, WHITE); 
      } else {
        display.drawLine(px, py + 4, px - 15, py - 10, WHITE); 
        display.fillTriangle(px - 17, py + 5, px - 13, py + 5, px - 15, py + 12, WHITE); 
      }
    } else {
      rod[0] = {(float)px + 8, (float)py + 4};
      for (int i = 1; i < segments; i++) {
        float angle = (map(i, 0, segments, -10, 5) / 10.0) + (sin(t * 1.5 + i * 0.3) * 0.1);
        rod[i].x = rod[i-1].x + cos(angle) * 3;
        rod[i].y = rod[i-1].y + sin(angle) * 3;
        display.drawPixel((int)rod[i].x, (int)rod[i].y, WHITE);
      }
      display.drawLine((int)rod[segments-1].x, (int)rod[segments-1].y, (int)rod[segments-1].x + 8, 50 + sin(t * 2) * 3, WHITE);
    }
  }
  display.display();
  t += 0.1;
}

void setup() {
  Serial.begin(115200);
  delay(1000); 

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("Erro no OLED"); 
    for(;;); 
  }
  
  // Comunicação com DFPlayer (RX2=16, TX2=17 no ESP32)
  somSerial.begin(9600, SERIAL_8N1, 16, 17);
  for (int i = 0; i < 5; i++) { pinMode(pinosLDR[i], INPUT); }
  randomSeed(analogRead(0)); 
  
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  servoTampas.setPeriodHertz(50);
  servoTampas.attach(pinoServoTampas, 500, 2400);
  
  Serial.println("Testando Servo Tampas...");
  servoTampas.write(180); 
  delay(1200);
  servoTampas.write(0);   
  delay(1200);
  servoTampas.detach();   

  Serial.println("Iniciando DFPlayer...");
  if (myDFPlayer.begin(somSerial)) { 
    myDFPlayer.volume(25); 
    delay(500);
    myDFPlayer.play(1);
    Serial.println("Audio 1 a tocar!");
  } else {
    Serial.println("Erro ao conectar com DFPlayer! Verifica cartao SD e fios.");
  }
}

void loop() {
  desenharAnimacaoPescador();

  unsigned long agora = millis();
  static bool aguardandoConfirmacao = false;
  static unsigned long tempoInicioToque = 0;
  static unsigned long tempoDebug = 0;

  //ldrs e som
  if (!sistemaFinalizado && !aguardandoServo) {
    int leituraLDR = analogRead(pinosLDR[etapaAtual]);
    
    if (agora - tempoDebug > 500) {
      Serial.print("Etapa Atual: "); Serial.print(etapaAtual);
      Serial.print(" | Pino: "); Serial.print(pinosLDR[etapaAtual]);
      Serial.print(" | Valor LDR: "); Serial.println(leituraLDR);
      tempoDebug = agora;
    }

    if (leituraLDR > limiteEscuro) {
      if (!aguardandoConfirmacao) { 
        tempoInicioToque = agora; 
        aguardandoConfirmacao = true; 
        Serial.println("LDR detetado! Mantem tapado...");
      } 
      else if (agora - tempoInicioToque >= 800) {
        etapaAtual++;
        aguardandoConfirmacao = false;
        Serial.print("Confirmado! Novo Audio a tocar: "); Serial.println(etapaAtual + 1);

        if (etapaAtual == 1) myDFPlayer.play(2);      
        else if (etapaAtual == 2) myDFPlayer.play(3); 
        else if (etapaAtual == 3) myDFPlayer.play(4); 
        else if (etapaAtual == 4) myDFPlayer.play(5); 
        else if (etapaAtual == 5) {                   
          myDFPlayer.play(6);                         
          tempoEsperaPosAudio = agora;                
          aguardandoServo = true; 
          Serial.println("Ultimo LDR! Iniciando espera final...");
        }
      }
    } else { 
      aguardandoConfirmacao = false; 
    }
  } 

  // Parte da festa final
  if (aguardandoServo && !sistemaFinalizado) {
    if (agora - tempoEsperaPosAudio >= 5000) { 
      sistemaFinalizado = true; 
      aguardandoServo = false;
      tempoVitoria = agora;
      tempoProximoFogo = agora + 500;
      
      servoPeixe.attach(pinoServoPeixe, 500, 2400);
      tempoServo = agora; 
      estadoServo = 1; 
      contadorAbanar = 0;
      Serial.println("Animacao Final e Servo Ativados!");
    }
  }

  // peixe a abanar
  if (sistemaFinalizado && !movimentoConcluido) {
    if (estadoServo == 1) { // esquerda
      servoPeixe.write(160);
      if (agora - tempoServo >= 300) { estadoServo = 2; tempoServo = agora; }
    }
    else if (estadoServo == 2) { //direita
      servoPeixe.write(20);
      if (agora - tempoServo >= 300) { 
        contadorAbanar++;
        tempoServo = agora;
        if (contadorAbanar >= 8) estadoServo = 3; 
        else estadoServo = 1;
      }
    }
    else if (estadoServo == 3) {
      servoPeixe.write(90);
      if (agora - tempoServo >= 500) { 
        servoPeixe.detach(); 
        estadoServo = 0; 
        movimentoConcluido = true; 
      }
    }
  }
}