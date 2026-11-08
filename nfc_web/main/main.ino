#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>

// Importamos nuestra configuración y diseño web
#include "config.h"
#include "web_nfc.hpp"

// Instancias de hardware
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
WebServer server(80);

// Variables de los sensores
int valLuz = 0;
int valSuelo = 0;
float tempAmbiente = 0.0;
float humAmbiente = 0.0;
bool estadoBomba = false;

// Control de tiempo sin usar delay()
unsigned long ultimoMuestreo = 0;
const long intervaloMuestreo = 2000; // 2 segundos

// Función para manejar las peticiones HTTP del móvil
void peticionNFC() {
  String htmlFinal = generarHTML(tempAmbiente, humAmbiente, valSuelo, valLuz, estadoBomba, UMBRAL_SUELO_SECO);
  server.send(200, "text/html", htmlFinal);
}

void setup() {
  Serial.begin(115200);

  // 1. Configuración de Pines
  pinMode(LDRPIN, INPUT);
  pinMode(SOILPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(RELAYPIN, OUTPUT);
  pinMode(BUZZERPIN, OUTPUT);

  // Asegurar actuadores apagados al inicio
  digitalWrite(RELAYPIN, HIGH); // Relés suelen ser Low-Level-Trigger
  digitalWrite(LEDPIN, LOW);
  noTone(BUZZERPIN);

  // 2. Iniciar Sensores y Pantalla
  dht.begin();
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Fallo en la pantalla OLED");
    while(true);
  }

  // 3. Crear el Punto de Acceso Wi-Fi (SoftAP) para el NFC
  WiFi.softAP(ssid_AP, password_AP);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Punto de Acceso Iniciado. IP para NFC: ");
  Serial.println(myIP); // Debe ser 192.168.4.1

  // 4. Iniciar Servidor Web
  server.on("/", peticionNFC);
  server.begin();

  // Pantalla de bienvenida
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("INICIANDO SISTEMA...");
  display.setCursor(0, 30);
  display.print("IP: "); display.println(myIP);
  display.display();
  delay(2000);
}

void loop() {
  // Atiende conexiones del móvil entrantes a través de la pegatina NFC
  server.handleClient();

  // Muestreo de datos temporizado (No bloqueante)
  unsigned long tiempoActual = millis();
  if (tiempoActual - ultimoMuestreo >= intervaloMuestreo) {
    ultimoMuestreo = tiempoActual;

    // --- LEER SENSORES ---
    valLuz = analogRead(LDRPIN);
    valSuelo = analogRead(SOILPIN);
    humAmbiente = dht.readHumidity();
    tempAmbiente = dht.readTemperature();

    if (isnan(humAmbiente) || isnan(tempAmbiente)) {
      Serial.println("Error de lectura DHT11");
      return; // Salta este ciclo si falla el DHT
    }

    // --- LÓGICA DE ACTUADORES ---
    // 1. LED de apoyo
    if (valLuz > UMBRAL_LUZ) {
      digitalWrite(LEDPIN, HIGH);
    } else {
      digitalWrite(LEDPIN, LOW);
    }

    // 2. Riego (Relé) y Alarma (Zumbador)
    if (valSuelo < UMBRAL_SUELO_SECO) {
      estadoBomba = true;
      actualizarOLED(); // Forzar actualización para mostrar "REGANDO"
      
      // Alarma (Tono bitonal suave de aviso)
      tone(BUZZERPIN, 800, 150); 
      delay(200); 
      tone(BUZZERPIN, 1200, 150);

      // Activar riego (Bloqueante controladamente)
      digitalWrite(RELAYPIN, LOW); // Enciende la bomba
      delay(TIEMPO_RIEGO_MS);
      digitalWrite(RELAYPIN, HIGH); // Apaga la bomba
      
      estadoBomba = false;
      noTone(BUZZERPIN);
    }

    // --- ACTUALIZAR PANTALLA OLED ---
    actualizarOLED();
  }
}

void actualizarOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  
  display.setCursor(0, 0); display.println("PLANTA ARA\xA4\x41"); // Araña
  display.println("----------------");
  
  display.setCursor(0, 16); display.print("Temp:  "); display.print(tempAmbiente, 1); display.println("C");
  display.setCursor(0, 26); display.print("Suelo: "); display.println(valSuelo);
  display.setCursor(0, 36); display.print("Luz:   "); display.println(valLuz);
  
  display.setCursor(0, 52); 
  if (estadoBomba) {
    display.println(">> REGANDO <<");
  } else {
    display.println("ESTADO: OK");
  }
  
  display.display();
}