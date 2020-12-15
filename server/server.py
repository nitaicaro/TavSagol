import sys
import json
import requests
from http.server import HTTPServer, BaseHTTPRequestHandler
from tinydb import TinyDB

# NETWORK CONFIGURATIONS #
PORT = 8080

# RESPONSE CODES #
CODE_FULL = "FULL"
CODE_NOT_FULL = "OK"
CODE_ERROR = "ERROR"
IP_RECEIVED = "IP RECEIVED"
MAX_UPDATED = "MAX UPDATED"

# DB CONFIGURATIONS #
DEFAULT_MAX_PEOPLE = 4
db = TinyDB('server/data.json')

subscribers = set()

class Serv(BaseHTTPRequestHandler):
    def do_POST(self):
        print("POST Received from ")
        print('----------')
        content_length = int(self.headers.get('content-length', 0))
        post_data = json.loads(self.rfile.read(content_length))
        response = self.handlePostData(post_data)
        self.writeResponse(response)
        if response is CODE_FULL or response is CODE_NOT_FULL:
            broadcastAllUnits(response)
        print("Current subscribers:")
        print(subscribers)
        print('-----------------------------------')

    def writeResponse(self, content):
        self.send_response(200)
        self.end_headers()
        self.wfile.write(content.encode())
    
    def handlePostData(self, post_data):
        if 'amount' in post_data:
            amount = post_data['amount']
            return updateCounter(amount)
        elif 'ip' in post_data:
            subscribers.add(self.client_address[0])
            return IP_RECEIVED
        elif 'max' in post_data:
            updateMax(post_data['max'])
            print("MAX WAS CHANGED TO " + str(post_data['max']))
            return MAX_UPDATED
        else:
            print('post data is ' + post_data)
            print('----------')
        
def getField(field):
    try:
        return db.all()[0][field]
    except IndexError or KeyError: 
        # db hasn't been created
        db.insert({'counter': 0})
        db.insert({'max': DEFAULT_MAX_PEOPLE})
    return db.all()[0][field]

def updateCounter(amount):
    if (not checkInt(amount)):
        return CODE_ERROR
    current_counter = getField('counter')
    new_counter = current_counter + int(amount) 
    db.update({'counter': new_counter})
    if (new_counter >= int(getField('max'))):
        return CODE_FULL
    return CODE_NOT_FULL

def updateMax(max):
    db.update({'max': max})

def checkInt(s):
    if len(s) == 0:
        return False
    if s[0] in ('-', '+'):
        return len(s) > 1 and s[1:].isdigit()
    return s.isdigit()
    
def broadcastAllUnits(response):
    for subscriber in subscribers:
        url = 'http://' + subscriber + ':80/'
        x = requests.post(url = 'http://18.221.133.197:8080', data = {"response": response})
    print('----------')

print("SERVER ON....")
print('----------')
httpd = HTTPServer(("", PORT), Serv)
httpd.serve_forever()
