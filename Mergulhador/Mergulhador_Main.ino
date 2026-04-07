#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "DFRobotDFPlayerMini.h"

const int DURACAO_AUDIO_5 = 28; 
const int DURACAO_AUDIO_6 = 20; 
const int DURACAO_AUDIO_7 = 16;  

#define TRIG_PIN 18
#define ECHO_PIN 19
#define PIN_BOMBA 14
#define RX_SOM 17 
#define TX_SOM 16 

Adafruit_8x8matrix olhoEsq = Adafruit_8x8matrix();
Adafruit_8x8matrix olhoDir = Adafruit_8x8matrix();
HardwareSerial somSerial(2); 
DFRobotDFPlayerMini myDFPlayer;

// --- BITMAPS ---
static const uint8_t PROGMEM eyeopen_bmp[] = {0x1E, 0x21, 0x21, 0x2D, 0x2D, 0x21, 0x21, 0x1E};
static const uint8_t PROGMEM eyeclosed_bmp[] = {0x00, 0x00, 0x00, 0x3F, 0x3F, 0x00, 0x00, 0x00};
static const uint8_t PROGMEM olhar_dir_bmp[] = {0x1E, 0x21, 0x21, 0x27, 0x27, 0x21, 0x21, 0x1E};
static const uint8_t PROGMEM joyEsq[] = {0x03, 0x06, 0x0C, 0x18, 0x18, 0x0C, 0x06, 0x03}; 
static const uint8_t PROGMEM joyDir[] = {0x60, 0x30, 0x18, 0x0C, 0x0C, 0x18, 0x30, 0x60};
static const uint8_t PROGMEM num3[] = {0x1F, 0x01, 0x01, 0x1F, 0x01, 0x01, 0x1F, 0x00};
static const uint8_t PROGMEM num2[] = {0x1F, 0x10, 0x10, 0x1F, 0x01, 0x01, 0x1F, 0x00};
static const uint8_t PROGMEM num1[] = {0x00, 0x1C, 0x08, 0x08, 0x08, 0x08, 0x18, 0x08};

bool emExecucao = false;

void setup() {
  Serial.begin(115200);
  somSerial.begin(9600, SERIAL_8N1, RX_SOM, TX_SOM);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIN_BOMBA, OUTPUT);
  digitalWrite(PIN_BOMBA, LOW);
  
  Wire.begin(21, 22); 
  olhoEsq.begin(0x70); olhoDir.begin(0x71);
  olhoEsq.setBrightness(1); olhoDir.setBrightness(1);

  delay(2000); 
  if (!myDFPlayer.begin(somSerial, false, false)) { Serial.println("Erro DFPlayer!"); }
  myDFPlayer.volume(25); 
}

void mostrar(const uint8_t *bmpE, const uint8_t *bmpD) {
  olhoEsq.clear(); olhoDir.clear();
  olhoEsq.drawBitmap(0, 0, bmpE, 8, 8, LED_ON);
  olhoDir.drawBitmap(0, 0, bmpD, 8, 8, LED_ON);
  olhoEsq.writeDisplay(); olhoDir.writeDisplay();
}

long lerDistancia() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duracao = pulseIn(ECHO_PIN, HIGH, 30000); 
  long dist = duracao * 0.034 / 2;
  return (dist <= 0) ? 999 : dist;
}

void executarCiclo() {
  emExecucao = true;
  unsigned long tempoInicio;

  // Etapa 1
  myDFPlayer.playMp3Folder(5);
  tempoInicio = millis();
  bool bombaAtivada = false;

  while (millis() - tempoInicio < (DURACAO_AUDIO_5 * 1000)) {
    unsigned long decorrido = millis() - tempoInicio;
    if (decorrido >= 19000 && !bombaAtivada) {
      mostrar(olhar_dir_bmp, olhar_dir_bmp);
      if (lerDistancia() < 10) {
        digitalWrite(PIN_BOMBA, HIGH);
        delay(1500);
        digitalWrite(PIN_BOMBA, LOW);
        bombaAtivada = true;
        mostrar(eyeopen_bmp, eyeopen_bmp);
      }
    } else {
      mostrar(eyeopen_bmp, eyeopen_bmp); delay(800);
      mostrar(eyeclosed_bmp, eyeclosed_bmp); delay(300);
    }
  }

  // Etapa 3
  myDFPlayer.playMp3Folder(6);
  tempoInicio = millis();
  bool contagemFeita = false;

  while (millis() - tempoInicio < (DURACAO_AUDIO_6 * 1000)) {
    unsigned long decorrido = millis() - tempoInicio;
    if (decorrido >= 17000 && !contagemFeita) {
      mostrar(num3, num3); delay(1000);
      mostrar(num2, num2); delay(1000);
      mostrar(num1, num1); delay(1000);
      contagemFeita = true;
    } else {
      mostrar(joyEsq, joyDir); delay(800);
      mostrar(eyeclosed_bmp, eyeclosed_bmp); delay(300);
    }
  }

  // Etapa 4
  myDFPlayer.playMp3Folder(7);
  tempoInicio = millis();
  
  while (millis() - tempoInicio < (DURACAO_AUDIO_7 * 1000)) {
    
    if ((millis() - tempoInicio) > ((DURACAO_AUDIO_7 - 5) * 1000)) {
       mostrar(joyEsq, joyDir);
       delay(100); 
    } else {
       
       mostrar(eyeopen_bmp, eyeopen_bmp); delay(800);
       mostrar(eyeclosed_bmp, eyeclosed_bmp); delay(300);
    }
  }
  
  mostrar(joyEsq, joyDir);
  delay(1000); 

  emExecucao = false;
}

void loop() {
  long d = lerDistancia();
  if (!emExecucao) {
    if (d < 60) {
      executarCiclo();
    } else {
      mostrar(eyeclosed_bmp, eyeclosed_bmp);
    }
  }
  delay(200);
}