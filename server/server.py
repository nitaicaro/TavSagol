import sys
import json
from http.server import HTTPServer, BaseHTTPRequestHandler
from tinydb import TinyDB

# NETWORK CONFIGURATIONS #
PORT = 8080

# RESPONSE CODES #
CODE_FULL = "FULL"
CODE_NOT_FULL = "OK"
CODE_ERROR = "ERROR"

# DB CONFIGURATIONS #
MAX_PEOPLE = 4
db = TinyDB('server/data.json')

class Serv(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get('content-length', 0))
        post_data = json.loads(self.rfile.read(content_length))
        self.handlePostData(post_data)

    def writeResponse(self, content):
        self.send_response(200)
        self.end_headers()
        self.wfile.write(content.encode())
    
    def handlePostData(self, post_data):
        if 'amount' in post_data:
            amount = post_data['amount']
            response = updateCounter(amount)
        elif 'image' in post_data:
            pass
        self.writeResponse(response)
    
def getField(field):
    try:
        return db.all()[0][field]
    except IndexError: 
        # db hasn't been created
        db.insert({'counter': 0})
    return db.all()[0][field]

def updateCounter(amount):
    if (not checkInt(amount)):
        return CODE_ERROR
    current_counter = getField('counter')
    new_counter = current_counter + int(amount) 
    db.update({'counter': new_counter})
    if (new_counter >= MAX_PEOPLE):
        return CODE_FULL
    return CODE_NOT_FULL

def checkInt(s):
    if len(s) == 0:
        return False
    if s[0] in ('-', '+'):
        return len(s) > 1 and s[1:].isdigit()
    return s.isdigit()
    

httpd = HTTPServer(("", PORT), Serv)
httpd.serve_forever()
