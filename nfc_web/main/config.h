#ifndef CONFIG_H
#define CONFIG_H

// --- PINOUT ESP32 ---
#define DHTPIN       27
#define LDRPIN       34
#define SOILPIN      33

// Actuadores
#define LEDPIN       25   // Luz de apoyo
#define RELAYPIN     26   // Bomba de agua (Relé)
#define BUZZERPIN    14   // Alarma acústica pasiva
                          // Pantalla OLED I2C: SDA=21, SCL=22

#define DHTTYPE      DHT11 

// --- UMBRALES DE CONTROL ---
#define UMBRAL_LUZ         2000  // Valor ADC para encender LED auxiliar
#define UMBRAL_SUELO_SECO   400  // Límite de humedad de la Planta Araña
#define TIEMPO_RIEGO_MS    3000  // Duración del riego automático

// --- CONFIGURACIÓN WIFI AP (Para NFC) ---
const char* ssid_AP = "Estacion_Planta_UAH"; // Red que emite el ESP32
const char* password_AP = "12345678";

#endif