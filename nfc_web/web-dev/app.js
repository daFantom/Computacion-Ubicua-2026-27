// app.js — cliente web que habla MQTT por WebSocket directamente con Mosquitto
const cfg = window.CONFIG;
const $ = (id) => document.getElementById(id);
const state = { mqtt: null, chart: null, history: [], pendientes: new Map() };

// ---------- UI helpers ----------
function setPill(id, estado, texto) {
  const el = $(id);
  if (!el) return;
  el.className = 'pill ' + (estado || '');
  el.innerHTML = `<span class="dot"></span> ${texto}`;
}
function fmt(v, dec = 1) { return (v === null || v === undefined || isNaN(v)) ? '—' : Number(v).toFixed(dec); }
function siNo(v) { return v ? 'Sí' : 'No'; }
function logEvent(tipo, texto) {
  const log = $('log');
  const el = document.createElement('div');
  const ts = new Date().toLocaleTimeString();
  el.innerHTML = `<span class="ts">[${ts}]</span> <span class="${tipo}">${texto}</span>`;
  log.appendChild(el);
  log.scrollTop = log.scrollHeight;
  while (log.childElementCount > 200) log.removeChild(log.firstChild);
}

// ---------- Render ----------
function renderSensores(d) {
  $('v-temp').textContent  = d.temperatura != null ? fmt(d.temperatura, 1) : '—';
  $('v-hum').textContent   = d.humedad_aire != null ? fmt(d.humedad_aire, 1) : '—';
  $('v-suelo').textContent = d.humedad_suelo != null ? String(d.humedad_suelo) : '—';
  $('v-luz').textContent   = d.luz != null ? String(d.luz) : '—';
  $('kpi-suelo').classList.toggle('alerta', d.humedad_suelo != null && d.humedad_suelo < 20);
  pushChart(d);
}

function renderEstado(e) {
  $('s-bomba').textContent    = siNo(e.bomba);  $('s-bomba').className   = 'v ' + (e.bomba ? 'on' : 'off');
  $('s-origen').textContent   = e.origen_bomba || '—';
  $('s-led').textContent      = siNo(e.led);    $('s-led').className     = 'v ' + (e.led ? 'on' : 'off');
  $('s-modoled').textContent  = e.modo_led || '—';
  $('s-zumb').textContent     = siNo(e.zumbador);$('s-zumb').className   = 'v ' + (e.zumbador ? 'alerta' : 'off');
  $('s-modoriego').textContent= e.modo_riego || '—';
  $('s-usuelo').textContent   = e.umbral_suelo != null ? e.umbral_suelo + ' %' : '—';
  $('s-uluz').textContent     = e.umbral_luz != null ? e.umbral_luz : '—';
  $('s-rssi').textContent     = e.wifi_rssi != null ? e.wifi_rssi + ' dBm' : '—';
  if (e.umbral_suelo != null) $('in-usuelo').value = e.umbral_suelo;
  if (e.umbral_luz   != null) $('in-uluz').value   = e.umbral_luz;
}

// ---------- Chart ----------
function initChart() {
  const ctx = $('chart').getContext('2d');
  state.chart = new Chart(ctx, {
    type: 'line',
    data: { labels: [], datasets: [
      { label: 'Humedad suelo (%)', data: [], borderColor: '#2e7d32', backgroundColor: 'rgba(46,125,50,.10)', tension: .3, fill: true,  yAxisID: 'y',  spanGaps: true },
      { label: 'Humedad aire (%)',  data: [], borderColor: '#1976d2', backgroundColor: 'rgba(25,118,210,.05)', tension: .3, fill: false, yAxisID: 'y',  spanGaps: true },
      { label: 'Temperatura (°C)',  data: [], borderColor: '#ef6c00', backgroundColor: 'rgba(239,108,0,.05)',  tension: .3, fill: false, yAxisID: 'y1', spanGaps: true },
    ]},
    options: {
      responsive: true, maintainAspectRatio: false, animation: false,
      interaction: { mode: 'index', intersect: false },
      plugins: { legend: { position: 'bottom' } },
      scales: {
        y:  { position: 'left',  min: 0, max: 100, title: { display: true, text: '%' } },
        y1: { position: 'right', min: 10, max: 40, grid: { drawOnChartArea: false }, title: { display: true, text: '°C' } },
        x:  { ticks: { maxTicksLimit: 8, autoSkip: true } }
      }
    }
  });
}
function pushChart(d) {
  if (!state.chart) return;
  state.history.push(d);
  if (state.history.length > 60) state.history.shift();
  state.chart.data.labels = state.history.map(x => x.ts ? new Date(x.ts).toLocaleTimeString() : '');
  state.chart.data.datasets[0].data = state.history.map(x => x.humedad_suelo ?? null);
  state.chart.data.datasets[1].data = state.history.map(x => x.humedad_aire ?? null);
  state.chart.data.datasets[2].data = state.history.map(x => x.temperatura ?? null);
  state.chart.update('none');
}

// ---------- MQTT ----------
function conectarMQTT() {
  logEvent('info', `conectando a ${cfg.MQTT_WS_URL}…`);
  const c = mqtt.connect(cfg.MQTT_WS_URL, {
    clientId: 'web-' + Math.random().toString(16).slice(2, 10),
    reconnectPeriod: 3000, clean: true
  });
  state.mqtt = c;

  c.on('connect', () => {
    setPill('pill-mqtt', 'ok', 'MQTT OK');
    c.subscribe([cfg.TOPIC_SENSORES, cfg.TOPIC_ESTADO, cfg.TOPIC_ACK, cfg.TOPIC_DISPONIBILIDAD], { qos: 1 });
    // estado retenido por si acaba de arrancar
    fetch(cfg.API_BASE + '/api/estado').then(r => r.json()).then(e => Object.keys(e).length && renderEstado(e)).catch(() => {});
    cargarHistorico();
  });
  c.on('reconnect', () => setPill('pill-mqtt', 'warn', 'Reconectando…'));
  c.on('close',    () => setPill('pill-mqtt', 'err',  'MQTT caído'));
  c.on('error', e => logEvent('ko', 'MQTT: ' + e.message));

  c.on('message', (topic, payload) => {
    const raw = payload.toString();
    let data;
    try { data = JSON.parse(raw); } catch { return; }
    if (topic === cfg.TOPIC_SENSORES)      renderSensores({ ...data, ts: new Date().toISOString() });
    else if (topic === cfg.TOPIC_ESTADO)   renderEstado(data);
    else if (topic === cfg.TOPIC_ACK)      onAck(data);
    else if (topic === cfg.TOPIC_DISPONIBILIDAD) {
      setPill('pill-dispo', raw === 'online' ? 'ok' : 'err', raw === 'online' ? 'ESP32 online' : 'ESP32 offline');
      logEvent('info', `disponibilidad: ${raw}`);
    }
  });
}

// ---------- Envío de comandos (por MQTT, igual que la web "oficial") ----------
function enviarComando(payload, btn) {
  if (!state.mqtt || !state.mqtt.connected) {
    logEvent('ko', 'Sin MQTT, no se puede enviar');
    if (btn) btn.disabled = false;
    return;
  }
  const id = 'web-' + Date.now() + '-' + Math.floor(Math.random() * 1000);
  payload.id = id;
  if (btn) btn.disabled = true;
  state.pendientes.set(id, btn);
  state.mqtt.publish(cfg.TOPIC_COMANDOS, JSON.stringify(payload), { qos: 1 });
  logEvent('info', `→ ${JSON.stringify(payload)}`);
  setTimeout(() => {
    if (state.pendientes.has(id)) {
      const b = state.pendientes.get(id);
      if (b) b.disabled = false;
      state.pendientes.delete(id);
      logEvent('ko', `sin ACK para ${id}`);
    }
  }, 4000);
}

function onAck(ack) {
  logEvent(ack.ok ? 'ok' : 'ko', `← ACK ${ack.id}: ${ack.detalle}`);
  const btn = state.pendientes.get(ack.id);
  if (btn) { btn.disabled = false; state.pendientes.delete(ack.id); }
}

// ---------- Histórico desde el backend del compañero ----------
async function cargarHistorico() {
  try {
    const r = await fetch(cfg.API_BASE + '/api/sensores?limit=60');
    if (!r.ok) return;
    const rows = await r.json();
    rows.forEach(d => pushChart(d));
    logEvent('info', `histórico: ${rows.length} filas`);
  } catch (e) {
    logEvent('ko', 'histórico no disponible: ' + e.message);
  }
}

// ---------- Bindings ----------
function bind() {
  $('btn-riego')   .onclick = e => enviarComando({ actuador: 'bomba', accion: 'on', duracion_s: 5 }, e.target);
  $('btn-stop')    .onclick = e => enviarComando({ actuador: 'bomba', accion: 'off' }, e.target);
  $('btn-led-on')  .onclick = e => enviarComando({ actuador: 'led', accion: 'on' }, e.target);
  $('btn-led-off') .onclick = e => enviarComando({ actuador: 'led', accion: 'off' }, e.target);
  $('btn-led-auto').onclick = e => enviarComando({ actuador: 'led', accion: 'auto' }, e.target);
  $('btn-buzz')    .onclick = e => enviarComando({ actuador: 'zumbador', accion: 'test' }, e.target);
  $('btn-modo-auto').onclick = e => enviarComando({ modo_riego: 'auto' }, e.target);
  $('btn-modo-man') .onclick = e => enviarComando({ modo_riego: 'manual' }, e.target);
  $('btn-aplicar') .onclick = e => enviarComando({
    umbral_suelo: parseInt($('in-usuelo').value),
    umbral_luz:   parseInt($('in-uluz').value)
  }, e.target);
}

// ---------- Init ----------
document.addEventListener('DOMContentLoaded', () => {
  initChart();
  bind();
  setPill('pill-dispo', '', 'Esperando ESP32');
  conectarMQTT();
});