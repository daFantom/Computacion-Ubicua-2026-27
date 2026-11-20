// config.js — lo único que cambia entre "dev local" y "producción en su backend"
window.CONFIG = {
  // MQTT sobre WebSocket: puerto 9001 del Mosquitto del compañero
  MQTT_WS_URL: 'ws://192.168.1.42:9001',

  // REST del backend del compañero (histórico, estado retenido)
  API_BASE: 'http://192.168.1.42:3000',

  TOPIC_SENSORES:       'invernadero/sensores',
  TOPIC_ESTADO:         'invernadero/estado',
  TOPIC_COMANDOS:       'invernadero/comandos',
  TOPIC_ACK:            'invernadero/ack',
  TOPIC_DISPONIBILIDAD: 'invernadero/disponibilidad',
};