#include <Wire.h>
#include <Servo.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define RUNTER_LED_PIN 13
#define HOCH_LED_PIN 12

#define SERVO_PIN 2

#define STATUS_AUSGANG 7
#define STATUS_EINGANG 8

Adafruit_BME280 bme;
Servo servo;

struct Messwert {
   float druck;
   float temperatur;
   float feuchte;
};

void messwerteAusgeben() {
  for (int i = 0; i < 1024 - sizeof(Messwert); i += sizeof(Messwert)) {
    struct Messwert m;
    EEPROM.get(i, m);

    Serial.println("------------------");
    Serial.print("Messung: ");
    Serial.println(i);
    
    Serial.print("Druck: ");
    Serial.println(m.druck);

    Serial.print("Temperatur: ");
    Serial.println(m.temperatur);

    Serial.print("Feuchte: ");
    Serial.println(m.feuchte);
  }
}

int speicherPosition = 0;

void messwertSchreiben(float druck, float temperatur, float feuchte) {
  if (speicherPosition >= 1024 - sizeof(Messwert)) {
    return;
  }
  
  struct Messwert m;
  m.druck = druck;
  m.temperatur = temperatur;
  m.feuchte = feuchte;

  EEPROM.put(speicherPosition, m);

  speicherPosition += sizeof(Messwert);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Verbindung hergestellt");
  
  pinMode(STATUS_AUSGANG, OUTPUT);
  digitalWrite(STATUS_AUSGANG, HIGH);
  pinMode(STATUS_EINGANG, INPUT);

  if (digitalRead(STATUS_EINGANG) == HIGH) {
    while (true) {
      messwerteAusgeben();
      Serial.println("Fertig!");
      delay(5000);
    } 
  }
  
  pinMode(RUNTER_LED_PIN, OUTPUT);
  pinMode(HOCH_LED_PIN, OUTPUT);

  delay(1000);
  
  bme.begin(0x76);

  delay(1000);
  
  servo.attach(SERVO_PIN);
  servo.write(0);

  delay(1000);
}

float letzterDruck = -1;
float kleinsterDruck = -1;

void loop() { 
  float druck = bme.readPressure();
  float temperatur = bme.readTemperature();
  float feuchte = bme.readHumidity();

  if (letzterDruck == -1) {
    // erste Messung
    
    kleinsterDruck = druck;
  } else {
    // LEDs steuern
    if (druck < letzterDruck) {
        Serial.print("Es geht nach oben");
        digitalWrite(HOCH_LED_PIN, HIGH);
        digitalWrite(RUNTER_LED_PIN, LOW);
    } else {
        Serial.print("Es geht nach unten");
        digitalWrite(RUNTER_LED_PIN, HIGH);
        digitalWrite(HOCH_LED_PIN, LOW);
    }

    // Druck ausgeben
    Serial.print(" Druck: ");
    Serial.println(druck);

    if (druck < kleinsterDruck) {
      kleinsterDruck = druck;
    }

    if (druck > kleinsterDruck + 12) {
      fallschirmOeffnen();
    }
  }

  messwertSchreiben(druck, temperatur, feuchte);
  
  letzterDruck = druck;
  
  delay(500);
}

void fallschirmOeffnen() {
  Serial.println("Fallschirm!");
  servo.write(180);
  delay(5000);
  exit(0);
}
