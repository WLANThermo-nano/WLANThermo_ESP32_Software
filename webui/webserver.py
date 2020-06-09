from http.server import HTTPServer, BaseHTTPRequestHandler

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        if self.path == '/':
            fileName = "index.html"
        elif self.path == '/WLANThermoLogo.png':
            fileName = "WLANThermoMini.png"
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
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)

        if self.path == '/setbluetooth':
            f = open('bluetooth', 'wb')
            f.write(body)
            self.send_response(200)
            self.end_headers()
        else:
            self.send_response(200)
            self.end_headers()
            f = open(self.path.replace('/', ''), 'rb')
            self.wfile.write(f.read())

httpd = HTTPServer(('localhost', 80), SimpleHTTPRequestHandler)
httpd.serve_forever()
