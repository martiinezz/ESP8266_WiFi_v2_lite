import http.server
import socketserver
import json
import re
import os

PORT = 8080
HEADER_FILE = "src/web_static/web_server_static_files.h"

def get_home_html():
    if not os.path.exists(HEADER_FILE):
        return "<html><body><h1>Error</h1><p>Could not find " + HEADER_FILE + "</p></body></html>"
    
    with open(HEADER_FILE, "r") as f:
        content = f.read()
        # Match R"=====( ... )====="
        match = re.search(r'CONTENT_HOME_HTML\[\] PROGMEM = R"=====\((.*?)\)=====";', content, re.DOTALL)
        if match:
            return match.group(1)
    return "<html><body><h1>Error</h1><p>Could not extract HTML from header</p></body></html>"

class MockHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/" or self.path == "/home.html":
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(get_home_html().encode("utf-8"))
        elif self.path.startswith("/status"):
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            status = {
                "mode": "STA",
                "wifi_client_connected": 1,
                "srssi": -50,
                "ipaddress": "192.168.1.100",
                "mqtt_connected": 1,
                "free_heap": 40000,
                "comm_sent": 100,
                "comm_success": 100,
                "rapi_connected": 1,
                "amp": 16000,
                "voltage": 230,
                "pilot": 32,
                "state": 2, # Charging
                "wh": 1234,
                "ota_update": 0
            }
            self.wfile.write(json.dumps(status).encode("utf-8"))
        elif self.path.startswith("/config"):
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            config = {
                "ssid": "MockWiFi",
                "mqtt_server": "mqtt.example.com",
                "mqtt_topic": "openevse-lite",
                "hostname": "openevse-lite"
            }
            self.wfile.write(json.dumps(config).encode("utf-8"))
        elif self.path.startswith("/rapi"):
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ret": "$OK", "cmd": "mock"}).encode("utf-8"))
        else:
            self.send_error(404)

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        print(f"POST to {self.path}: {post_data.decode('utf-8')}")
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.end_headers()
        self.wfile.write(json.dumps({"msg": "done"}).encode("utf-8"))

    def log_message(self, format, *args):
        # Suppress logging for cleaner output
        return

print(f"Starting OpenEVSE Lite GUI Simulator on http://localhost:{PORT}")
with socketserver.TCPServer(("", PORT), MockHandler) as httpd:
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down...")
        httpd.shutdown()
