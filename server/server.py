import sys
import json
import requests
import datetime
import ssl
from http.server import HTTPServer, BaseHTTPRequestHandler
from tinydb import TinyDB

# NETWORK CONFIGURATIONS #
PORT = 8080

# RESPONSE CODES #
CODE_FULL = "FULL"
CODE_NOT_FULL = "OK"
CODE_ERROR = "ERROR"
# IP_RECEIVED = "IP RECEIVED"
MAX_UPDATED = "MAX UPDATED"
INVALID_ENTRANCE_RECEIVED = "INVALID_ENTRANCE_RECEIVED" 
HIGH_TEMP_RECEIVED = "HIGH_TEMP_RECEIVED"

# DB CONFIGURATIONS #
DEFAULT_MAX_PEOPLE = 4
db = TinyDB('server/data.json')
history = TinyDB('server/history.json')


class Serv(BaseHTTPRequestHandler):

    def do_GET(self):
        print("GET Received from " + self.client_address[0])
        response = str(getField('counter')) + "," + str(getField('max'))
        self.writeResponse(response)

    def do_POST(self):
        content_length = int(self.headers.get('content-length', 0))
        post_data = json.loads(self.rfile.read(content_length))
        response = self.handlePostData(post_data)
        print("POST Received from " + self.client_address[0] + " with data ")
        print(post_data)
        print("RESPONDING TO POST WITH " + response)
        self.writeResponse(response)
        print('-----------------------------------')

    def writeResponse(self, content):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(content.encode())
    
    def handlePostData(self, post_data):
        if 'amount' in post_data:
            amount = post_data['amount']
            return updateCounter(amount)
        elif 'max' in post_data:
            updateMax(post_data['max'])
            print("MAX WAS CHANGED TO " + str(post_data['max']))
            return MAX_UPDATED
        elif 'invalid_temperature' in post_data:
            print("POST DATA IS")
            print(post_data)
            logHighTemp(post_data['invalid_temperature'])
            return HIGH_TEMP_RECEIVED
        elif 'history' in post_data:
            return json.dumps(history.all())
        elif 'invalid_entrance' in post_data:
            logHighTemp("ENTRANCE WITHOUT TEMPERATURE MEASUREMENT")
            return INVALID_ENTRANCE_RECEIVED
        else:
            print('post data is ' + post_data)
            print('----------')
        
def logHighTemp(temp):
    history.insert({str(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")): temp})

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

print("SERVER ON....")
print('----------')
httpd = HTTPServer(("", PORT), Serv)
httpd.serve_forever()
