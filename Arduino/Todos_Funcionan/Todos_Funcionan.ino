//Sensores
// Incluimos librería
#include <DHT.h>
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 27
#define LDRPIN 34
#define SOILPIN 33
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

//Rele
#define RELEPIN 26
//zumbador
#define BUZZERPIN 23
//Led
#define LEDPIN 25
#define UMBRAL_LUZ 2000
//Pantalla
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

int contador = 0;

void setup() {
  
  // Inicializamos comunicación serie
  Serial.begin(9600);
  // Comenzamos el sensor DHT
  dht.begin();
  pinMode(LDRPIN, INPUT);

  //Zumbador
  pinMode(BUZZERPIN, OUTPUT);
  //Rele
  pinMode(RELEPIN, OUTPUT);

  // Relé activado: bomba apagada
  digitalWrite(RELEPIN, HIGH);
  noTone(BUZZERPIN);

  //led
  pinMode(LEDPIN, OUTPUT);

  //Pantalla
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Error al iniciar OLED");
    while (true);
  }

  Serial.println("OLED funcionando");

  display.clearDisplay();
  display.display();
}

void loop() {
 // Esperamos 5 segundos entre medidas
  delay(5000);
  //Luz 4095 max, que es oscuridad
  //Darle con luz de movil se pone a 7 a cierta distancia
    int luz = analogRead(LDRPIN);

  Serial.print("Estado luminosidad: ");
  Serial.println(luz);

  // Control del LED y zumbador
  if (luz > UMBRAL_LUZ) {
      digitalWrite(LEDPIN, HIGH);
      // Activar relé: bomba encendida (NC)
      digitalWrite(RELEPIN, LOW);
      delay(2000);

      // Desactivar relé: bomba apagada
      digitalWrite(RELEPIN, HIGH);

      tone(BUZZERPIN, 1000, 1000);
  } else {
      digitalWrite(LEDPIN, LOW);
      noTone(BUZZERPIN);
}
  //Humedad 
  //Da unos 330-350 en el aire
  int humedad = analogRead(SOILPIN);

  Serial.print("Humedad del suelo: ");
  Serial.println(humedad);
  
  // Leemos la humedad relativa
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();
  // Leemos la temperatura en grados Fahrenheit
  float f = dht.readTemperature(true);
 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }
 
  // Calcular el índice de calor en Fahrenheit
  float hif = dht.computeHeatIndex(f, h);
  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(t, h, false);
 
  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  
  // Limpiamos la pantalla
display.clearDisplay();

display.setTextSize(1);
display.setTextColor(SSD1306_WHITE);

// Título
display.setCursor(0, 0);
display.println("SISTEMA DE RIEGO");

// Línea separadora
display.println("----------------");

// Luminosidad
display.setCursor(0, 18);
display.print("Luz: ");
display.println(luz);

// Humedad del suelo
display.setCursor(0, 30);
display.print("Suelo: ");
display.println(humedad);

// Humedad ambiental
display.setCursor(0, 42);
display.print("Ambiente: ");
display.print(h);
display.println("%");

// Temperatura
display.setCursor(0, 54);
display.print("Temp: ");
display.print(t);
display.println(" C");

// Actualizamos la pantalla
display.display();
}
