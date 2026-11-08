#ifndef WEB_NFC_H
#define WEB_NFC_H

#include <Arduino.h>

// Esta función devuelve el HTML completo sustituyendo las variables por sus valores actuales
String generarHTML(float temp, float hum, int suelo, int luz, bool bomba, int umbral) {
  String html = "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Monitor Planta Araña</title>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', sans-serif; background-color: #f4f7f6; color: #333; margin: 0; display: flex; flex-direction: column; align-items: center; }";
  html += ".header { background-color: #2E7D32; color: white; width: 100%; padding: 20px 0; text-align: center; margin-bottom: 20px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += ".header h1 { margin: 0; font-size: 1.5rem; } .header p { margin: 5px 0 0 0; font-size: 0.9rem; opacity: 0.9; }";
  html += ".dashboard { display: grid; grid-template-columns: 1fr; gap: 15px; width: 90%; max-width: 400px; }";
  html += ".card { background: white; border-radius: 12px; padding: 20px; box-shadow: 0 2px 10px rgba(0,0,0,0.05); display: flex; justify-content: space-between; align-items: center; border-left: 5px solid #2E7D32; }";
  html += ".card.alert { border-left-color: #d32f2f; }";
  html += ".card-info { display: flex; flex-direction: column; }";
  html += ".title { font-size: 0.85rem; color: #666; text-transform: uppercase; margin-bottom: 5px; }";
  html += ".value { font-size: 1.8rem; font-weight: bold; } .unit { font-size: 1rem; font-weight: normal; color: #666; }";
  html += ".icon { font-size: 2rem; opacity: 0.8; }";
  html += ".status { margin-top: 20px; padding: 15px; border-radius: 8px; width: 90%; max-width: 400px; text-align: center; font-weight: bold; background: #e8f5e9; color: #2E7D32; }";
  html += ".status.active { background: #ffebee; color: #d32f2f; }";
  html += ".footer { margin-top: 30px; font-size: 0.8rem; color: #666; text-align: center; padding-bottom: 20px; }";
  html += "</style></head><body>";

  html += "<div class='header'><h1>Monitor Planta Araña</h1><p>Grupo 1 - Computación Ubicua</p></div>";
  html += "<div class='dashboard'>";

  // Tarjetas de datos
  html += "<div class='card'><div class='card-info'><span class='title'>Temperatura</span><div class='value'>" + String(temp, 1) + " <span class='unit'>°C</span></div></div><div class='icon'>🌡️</div></div>";
  html += "<div class='card'><div class='card-info'><span class='title'>Humedad Aire</span><div class='value'>" + String(hum, 1) + " <span class='unit'>%</span></div></div><div class='icon'>💧</div></div>";
  
  String claseSuelo = (suelo < umbral) ? "card alert" : "card";
  html += "<div class='" + claseSuelo + "'><div class='card-info'><span class='title'>Humedad Suelo</span><div class='value'>" + String(suelo) + " <span class='unit'>/1023</span></div></div><div class='icon'>🌱</div></div>";
  
  html += "<div class='card'><div class='card-info'><span class='title'>Luz (LDR)</span><div class='value'>" + String(luz) + " <span class='unit'>lx</span></div></div><div class='icon'>☀️</div></div>";
  html += "</div>"; 

  // Alerta de bomba
  if (bomba) {
    html += "<div class='status active'> TIERRA SECA. Bomba de agua ACTIVADA.</div>";
  } else {
    html += "<div class='status'> Estado óptimo. Bomba inactiva.</div>";
  }

  html += "<div class='footer'>Conectado vía Wi-Fi local (ESP32)<br>Refresca la página para actualizar.</div>";
  html += "</body></html>";
  
  return html;
}
#endif