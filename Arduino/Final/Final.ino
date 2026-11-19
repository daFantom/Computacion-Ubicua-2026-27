
// SENSORES

#include <DHT.h>

#define DHTPIN 27
#define LDRPIN 34
#define SOILPIN 33

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ACTUADORES

// Rele
#define RELEPIN 26

// Zumbador
#define BUZZERPIN 23

// LED
#define LEDPIN 25

// Umbrales
#define UMBRAL_LUZ 2000
#define UMBRAL_HUMEDAD 1500

// Tiempo de activacion de la bomba
#define TIEMPO_RIEGO 2000

// PANTALLA OLED

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// VARIABLES

int luz = 0;
int humedadSuelo = 0;

float humedadAmbiente = 0;
float temperatura = 0;

bool pocaLuz = false;
bool sueloSeco = false;

// Control del riego
unsigned long tiempoInicioRiego = 0;
bool regando = false;

// SETUP

void setup() {

  Serial.begin(9600);

  // Sensores
  dht.begin();
  pinMode(LDRPIN, INPUT);

  // Actuadores
  pinMode(BUZZERPIN, OUTPUT);
  pinMode(RELEPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);

  // Estado inicial
  digitalWrite(RELEPIN, HIGH); // Bomba apagada
  digitalWrite(LEDPIN, LOW);
  noTone(BUZZERPIN);

  // Pantalla
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Error al iniciar OLED");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("SISTEMA DE RIEGO");
  display.println("Iniciando...");
  display.display();

  Serial.println("Sistema de riego iniciado");
}

// LOOP

void loop() {

  // LECTURA DE SENSORES
 
  luz = analogRead(LDRPIN);
  humedadSuelo = analogRead(SOILPIN);

  humedadAmbiente = dht.readHumidity();
  temperatura = dht.readTemperature();

  // COMPROBACION DE UMBRALES

  // Mucha oscuridad
  pocaLuz = (luz > UMBRAL_LUZ);

  // Tierra seca
  sueloSeco = (humedadSuelo > UMBRAL_HUMEDAD);

  // CONTROL DEL LED

  if (pocaLuz) {
    digitalWrite(LEDPIN, HIGH);
  } else {
    digitalWrite(LEDPIN, LOW);
  }

  // CONTROL DE LA BOMBA

  if (sueloSeco && !regando) {

    // Activar bomba
    digitalWrite(RELEPIN, LOW);

    tiempoInicioRiego = millis();
    regando = true;
  }

  // Desactivar bomba tras 2 segundos
  if (regando &&
      millis() - tiempoInicioRiego >= TIEMPO_RIEGO) {

    digitalWrite(RELEPIN, HIGH);
    regando = false;
  }

  // CONTROL DEL ZUMBADOR
 
  if (pocaLuz || sueloSeco) {
    tone(BUZZERPIN, 1000);
  } else {
    noTone(BUZZERPIN);
  }

  // MONITORIZACION SERIAL
  
  Serial.println("--------------------");

  Serial.print("Luminosidad: ");
  Serial.println(luz);

  Serial.print("Humedad suelo: ");
  Serial.println(humedadSuelo);

  if (!isnan(humedadAmbiente) && !isnan(temperatura)) {

    Serial.print("Humedad ambiente: ");
    Serial.print(humedadAmbiente);
    Serial.println(" %");

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" C");

  } else {
    Serial.println("Error leyendo DHT11");
  }

  // PANTALLA OLED
 
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("SISTEMA DE RIEGO");
  display.println("----------------");

  // Luminosidad
  display.setCursor(0, 18);
  display.print("Luz: ");
  display.println(luz);

  // Humedad suelo
  display.setCursor(0, 30);
  display.print("Suelo: ");
  display.println(humedadSuelo);

  // Humedad ambiental
  display.setCursor(0, 42);

  if (!isnan(humedadAmbiente)) {
    display.print("Ambiente: ");
    display.print(humedadAmbiente);
    display.println("%");
  } else {
    display.println("Ambiente: Error");
  }

  // Temperatura
  display.setCursor(0, 54);

  if (!isnan(temperatura)) {
    display.print("Temp: ");
    display.print(temperatura);
    display.println(" C");
  } else {
    display.println("Temp: Error");
  }

  display.display();

  // Esperar 2 segundos entre lecturas
  delay(2000);
}
