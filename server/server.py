import sys
import json
from http.server import HTTPServer, BaseHTTPRequestHandler
from tinydb import TinyDB

# NETWORK CONFIGURATIONS #
PORT = 8080

# RESPONSE CODES #
CODE_FULL = "FULL"
CODE_NOT_FULL = "OK"
ERROR = "ERROR"

# DB CONFIGURATIONS #
MAX_PEOPLE = 4

# GLOBAL VARS #
counter = 0

class Serv(BaseHTTPRequestHandler):
    def write_response(self, content):
        self.send_response(200)
        self.end_headers()
        self.wfile.write(content.encode())

    def do_POST(self):
        content_length = int(self.headers.get('content-length', 0))
        post_data = json.loads(self.rfile.read(content_length))
        amount = post_data["amount"]
        response = Serv.updateCounter(amount)
        self.write_response(response)

    def updateCounter(amount):
        if (not Serv.check_int(amount)):
            return ERROR
        db = TinyDB('data.json')
        current_counter = db.all()[0]['counter']
        new_counter = current_counter + int(amount) 
        db.update({'counter': new_counter})
        if (new_counter >= MAX_PEOPLE):
            return CODE_FULL
        return CODE_NOT_FULL

    def check_int(s):
        if len(s) == 0:
            return False
        if s[0] in ('-', '+'):
            return len(s) > 1 and s[1:].isdigit()
        return s.isdigit()


# updateCounter(sys.argv[1])

httpd = HTTPServer(("", PORT), Serv)
httpd.serve_forever()
