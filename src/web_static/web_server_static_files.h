const char CONTENT_HOME_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>OpenEVSE Lite</title>
  <style>
    body { font-family: sans-serif; background: #f4f4f4; margin: 0; padding: 10px; }
    .card { background: #fff; border-radius: 8px; padding: 15px; margin-bottom: 15px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h2 { margin-top: 0; color: #006666; }
    .status-val { font-size: 1.5em; font-weight: bold; }
    .btn { background: #006666; color: #fff; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; margin-right: 5px; }
    .btn:disabled { background: #ccc; }
    .btn-red { background: #c00; }
    label { display: block; margin-top: 10px; font-weight: bold; }
    input { width: 100%; padding: 8px; box-sizing: border-box; margin-top: 5px; }
    .hidden { display: none; }
    nav { margin-bottom: 15px; }
    nav button { background: none; border: none; color: #006666; cursor: pointer; font-size: 1.1em; padding: 5px 10px; }
    nav button.active { border-bottom: 2px solid #006666; font-weight: bold; }
    .timer-row { border-bottom: 1px solid #eee; padding: 10px 0; display: flex; justify-content: space-between; align-items: center; }
  </style>
</head>
<body>
  <nav>
    <button id="btn-dash" class="active" onclick="showPage('dash')">Dashboard</button>
    <button id="btn-sett" onclick="showPage('sett')">Settings</button>
  </nav>

  <div id="page-dash" class="card">
    <h2>Dashboard</h2>
    <div>State: <span id="state" class="status-val">?</span></div>
    <div>Amps: <span id="amp" class="status-val">0.0</span> A</div>
    <div>Pilot: <span id="pilot" class="status-val">0</span> A</div>
    <div>Energy: <span id="wh" class="status-val">0</span> Wh</div>
    <div style="margin-top:15px;">
      <button class="btn" onclick="rapi('$FE')">Resume</button>
      <button class="btn btn-red" onclick="rapi('$FS')">Pause</button>
    </div>
    <div style="margin-top:15px;">
      <label>Set Current (A):</label>
      <input type="number" id="set-amp" style="width:80px;" min="6" max="32" value="16">
      <button class="btn" onclick="rapi('$SC ' + document.getElementById('set-amp').value)">Set</button>
    </div>
  </div>

  <div id="page-sett" class="hidden">
    <div class="card">
      <h2>Timers</h2>
      <div id="timer-list"></div>
      <div style="margin-top:10px; border-top: 1px solid #ccc; padding-top:10px;">
        <label>New Timer (HH:MM):</label>
        <div style="display:flex; gap:5px;">
          <input type="time" id="t-start"> to <input type="time" id="t-end">
          <button class="btn" onclick="addTimer()">Add</button>
        </div>
      </div>
    </div>
    <div class="card">
      <h2>Wi-Fi Settings</h2>
      <label>SSID:</label><input id="wifi-ssid">
      <label>Pass:</label><input id="wifi-pass" type="password">
      <button class="btn" onclick="saveWifi()" style="margin-top:10px;">Save & Restart</button>
    </div>
    <div class="card">
      <h2>MQTT Settings</h2>
      <label>Server:</label><input id="mqtt-server">
      <label>Topic:</label><input id="mqtt-topic">
      <button class="btn" onclick="saveMqtt()" style="margin-top:10px;">Save</button>
    </div>
    <div class="card">
      <h2>System</h2>
      <button class="btn" onclick="syncTime()">Sync Browser Time</button>
      <button class="btn" onclick="fetch('/restart')">Restart</button>
      <button class="btn btn-red" onclick="if(confirm('Reset all?'))fetch('/reset')">Factory Reset</button>
      <hr>
      <a href="/update">OTA Update</a>
    </div>
  </div>

  <script>
    let ws;
    let timers = [];
    const states = ["Starting", "Ready", "Charging", "Waiting", "Fault", "Error"];

    function showPage(p) {
      document.getElementById('page-dash').classList.toggle('hidden', p !== 'dash');
      document.getElementById('page-sett').classList.toggle('hidden', p !== 'sett');
      document.getElementById('btn-dash').className = (p === 'dash' ? 'active' : '');
      document.getElementById('btn-sett').className = (p === 'sett' ? 'active' : '');
      if (p === 'sett') loadConfig();
    }

    function connect() {
      ws = new WebSocket('ws://' + location.host + '/ws');
      ws.onmessage = (e) => {
        const d = JSON.parse(e.data);
        updateStatus(d);
      };
      ws.onclose = () => setTimeout(connect, 2000);
    }

    function updateStatus(d) {
      if (d.state !== undefined) document.getElementById('state').innerText = states[d.state] || d.state;
      if (d.amp !== undefined) document.getElementById('amp').innerText = (d.amp/1000).toFixed(1);
      if (d.pilot !== undefined) document.getElementById('pilot').innerText = d.pilot;
      if (d.wh !== undefined) document.getElementById('wh').innerText = d.wh;
    }

    function rapi(c) {
      fetch('/rapi?json=1&rapi=' + encodeURIComponent(c)).then(r => r.json()).then(d => {
        if(d.error) alert(d.error);
      });
    }

    function syncTime() {
      const epoch = Math.floor(Date.now() / 1000);
      fetch('/time?epoch=' + epoch).then(r => r.json()).then(d => alert('Synced: ' + d.time));
    }

    function loadConfig() {
      fetch('/config').then(r => r.json()).then(d => {
        document.getElementById('wifi-ssid').value = d.ssid || '';
        document.getElementById('mqtt-server').value = d.mqtt_server || '';
        document.getElementById('mqtt-topic').value = d.mqtt_topic || '';
        timers = JSON.parse(d.timers || "[]");
        renderTimers();
      });
    }

    function renderTimers() {
      const list = document.getElementById('timer-list');
      list.innerHTML = timers.length ? '' : 'No timers set.';
      timers.forEach((t, i) => {
        const row = document.createElement('div');
        row.className = 'timer-row';
        row.innerHTML = `<span>${t.start} - ${t.end}</span><button class="btn btn-red" onclick="delTimer(${i})">Del</button>`;
        list.appendChild(row);
      });
    }

    function addTimer() {
      const start = document.getElementById('t-start').value;
      const end = document.getElementById('t-end').value;
      if (!start || !end) return;
      timers.push({start, end});
      saveTimers();
    }

    function delTimer(i) {
      timers.splice(i, 1);
      saveTimers();
    }

    function saveTimers() {
      const body = JSON.stringify({timers: JSON.stringify(timers)});
      fetch('/config', {method:'POST', body: body}).then(() => renderTimers());
    }

    function saveWifi() {
      const s = document.getElementById('wifi-ssid').value;
      const p = document.getElementById('wifi-pass').value;
      fetch('/savenetwork?ssid=' + encodeURIComponent(s) + '&pass=' + encodeURIComponent(p)).then(() => alert('Saved. Restarting...'));
    }

    function saveMqtt() {
      const s = document.getElementById('mqtt-server').value;
      const t = document.getElementById('mqtt-topic').value;
      const body = JSON.stringify({mqtt_server: s, mqtt_topic: t, mqtt_enabled: !!s});
      fetch('/config', {method:'POST', body: body}).then(() => alert('Saved'));
    }

    connect();
    setInterval(() => { fetch('/status').then(r=>r.json()).then(d => updateStatus(d))}, 5000);
    // Auto sync time on first load
    setTimeout(syncTime, 1000);
  </script>
</body>
</html>
)=====";

const char CONTENT_WIFI_PORTAL_HTML[] PROGMEM = "<html><body><h1>OpenEVSE WiFi Setup</h1><form method='get' action='savenetwork'>SSID: <input name='ssid'><br>Pass: <input name='pass' type='password'><br><input type='submit'></form></body></html>";
const char CONTENT_STYLE_CSS[] PROGMEM = "";
const char CONTENT_WIFI_PORTAL_JS[] PROGMEM = "";
const char CONTENT_FAVICON_16X16_PNG[] PROGMEM = "";
const char CONTENT_FAVICON_32X32_PNG[] PROGMEM = "";
const char CONTENT_WIFI_SIGNAL_1_SVG[] PROGMEM = "";
const char CONTENT_WIFI_SIGNAL_2_SVG[] PROGMEM = "";
const char CONTENT_WIFI_SIGNAL_3_SVG[] PROGMEM = "";
const char CONTENT_WIFI_SIGNAL_4_SVG[] PROGMEM = "";
const char CONTENT_WIFI_SIGNAL_5_SVG[] PROGMEM = "";

StaticFile staticFiles[] = {
  { "/home.html", CONTENT_HOME_HTML, sizeof(CONTENT_HOME_HTML) - 1, _CONTENT_TYPE_HTML },
  { "/style.css", CONTENT_STYLE_CSS, sizeof(CONTENT_STYLE_CSS) - 1, _CONTENT_TYPE_CSS },
  { "/wifi_portal.html", CONTENT_WIFI_PORTAL_HTML, sizeof(CONTENT_WIFI_PORTAL_HTML) - 1, _CONTENT_TYPE_HTML },
  { "/wifi_portal.js", CONTENT_WIFI_PORTAL_JS, sizeof(CONTENT_WIFI_PORTAL_JS) - 1, _CONTENT_TYPE_JS },
  { "/favicon-16x16.png", CONTENT_FAVICON_16X16_PNG, sizeof(CONTENT_FAVICON_16X16_PNG) - 1, _CONTENT_TYPE_PNG },
  { "/favicon-32x32.png", CONTENT_FAVICON_32X32_PNG, sizeof(CONTENT_FAVICON_32X32_PNG) - 1, _CONTENT_TYPE_PNG },
  { "/wifi_signal_1.svg", CONTENT_WIFI_SIGNAL_1_SVG, sizeof(CONTENT_WIFI_SIGNAL_1_SVG) - 1, _CONTENT_TYPE_SVG },
  { "/wifi_signal_2.svg", CONTENT_WIFI_SIGNAL_2_SVG, sizeof(CONTENT_WIFI_SIGNAL_2_SVG) - 1, _CONTENT_TYPE_SVG },
  { "/wifi_signal_3.svg", CONTENT_WIFI_SIGNAL_3_SVG, sizeof(CONTENT_WIFI_SIGNAL_3_SVG) - 1, _CONTENT_TYPE_SVG },
  { "/wifi_signal_4.svg", CONTENT_WIFI_SIGNAL_4_SVG, sizeof(CONTENT_WIFI_SIGNAL_4_SVG) - 1, _CONTENT_TYPE_SVG },
  { "/wifi_signal_5.svg", CONTENT_WIFI_SIGNAL_5_SVG, sizeof(CONTENT_WIFI_SIGNAL_5_SVG) - 1, _CONTENT_TYPE_SVG },
};
