from http.server import HTTPServer, BaseHTTPRequestHandler

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        if self.path == '/':
            fileName = "index.html"
        elif self.path == '/WLANThermoLogo.png':
            fileName = "WLANThermoMini.png"
        # elif self.path == '/networklist':
        #     body = b'{"Connect":true,"SSID":"WLAN-XXXXX","BSSID":"38:10:D5:D1:BC:8B","IP":"192.168.178.53","Mask":"255.255.255.0","Gate":"192.168.178.1","Scan":[{"SSID":"WLAN-XXXXX","BSSID":"38:10:D5:D1:BC:8B","RSSI":-42,"Enc":true},{"SSID":"EasyBox-767780","BSSID":"D4:60:E3:BD:BA:9C","RSSI":-66,"Enc":true},{"SSID":"FRITZ!Box 7590 LD","BSSID":"44:4E:6D:3B:0E:0F","RSSI":-76,"Enc":true}],"RSSI":-42,"Enc":true}'
        #     self.send_response(200)
        #     self.send_header( "Content-length", str(len(body)) )
        #     self.end_headers()
        #     self.wfile.write(body)
        #     return
        elif self.path == '/favicon.ico':
            self.send_response(404)
            self.end_headers()
            return
        else:
            fileName = self.path.replace('/', '')
        
        self.send_response(200)
        self.end_headers()
        f = open(fileName, 'rb')
        self.wfile.write(f.read())

    def do_POST(self):
        # content_length = int(self.headers['Content-Length'])
        # body = self.rfile.read(content_length)

        if self.path == '/setbluetooth':
            f = open('bluetooth', 'wb')
            f.write(body)
            self.send_response(200)
            self.end_headers()
        elif self.path == '/stopwifi':
            self.send_response(200)
            self.send_header( "Content-length", str(len("true")) )
            self.end_headers()
            self.wfile.write(b"true")
        else:
            self.send_response(200)
            self.end_headers()
            f = open(self.path.replace('/', ''), 'rb')
            self.wfile.write(f.read())

httpd = HTTPServer(('localhost', 80), SimpleHTTPRequestHandler)
httpd.serve_forever()
