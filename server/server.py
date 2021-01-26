import sys
import json
import requests
import datetime
import pytz
import os
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
MAX_UPDATE_FAILURE = "MAX UPDATE FAILURE"
INVALID_ENTRANCE_RECEIVED = "INVALID_ENTRANCE_RECEIVED" 
HIGH_TEMP_RECEIVED = "HIGH_TEMP_RECEIVED"
HISTORY_CLEARED = "HISTORY CLEARED"
HISTOGRAM_CLEARED = "HISTOGRAM_CLEARED"

# DB CONFIGURATIONS #
DEFAULT_MAX_PEOPLE = 4
db = TinyDB('server/data.json')
history = TinyDB('server/history.json')
times = TinyDB('server/times.json')

tz = pytz.timezone('Israel')

class Serv(BaseHTTPRequestHandler):

    def do_GET(self):
        print("GET Received from " + self.client_address[0])
        response = str(db.all()[0]['counter']) + "," + str(db.all()[0]['max'])
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
            return updateMax(post_data['max'])
        elif 'invalid_temperature' in post_data:
            logHighTemp("HIGH TEMPERATURE ENTRANCE: " + post_data['invalid_temperature'])
            return HIGH_TEMP_RECEIVED
        elif 'history' in post_data: 
            return json.dumps(history.all()[::-1])
        elif 'times' in post_data:
            return json.dumps(times.all())
        elif 'invalid_entrance' in post_data:
            logHighTemp("ENTRANCE WITHOUT TEMPERATURE MEASUREMENT")
            return INVALID_ENTRANCE_RECEIVED
        elif 'reset_history' in post_data:
            history.drop_tables()
            return HISTORY_CLEARED
        elif 'reset_histogram' in post_data:
            for hour in range(24):
                times.update({hour: 0})
            return HISTORY_CLEARED
        else:
            print('post data is ' + post_data)
            print('----------')
        
def logHighTemp(msg):
    history.insert({str(datetime.datetime.now(tz).strftime("%Y-%m-%d %H:%M:%S")): msg})

def updateCounter(amount):
    if (not checkInt(amount)):
        return CODE_ERROR
    current_counter = db.all()[0]['counter']
    new_counter = current_counter + int(amount) 
    db.update({'counter': new_counter})
    if int(amount) == 1:
        hour = datetime.datetime.now(tz).strftime("%H")
        current_value = times.all()[0][hour]
        new_value = current_value + 1 
        print("UPDATING " + str(hour) + "TO BE " + str(new_value))
        times.update({hour: new_value})
    if (new_counter >= int(db.all()[0]['max'])):
        if ((new_counter > int(db.all()[0]['max']) and (int(amount) == 1))):
            print("AMOUNT IS " + amount)
            history.insert({str(datetime.datetime.now(tz).strftime("%Y-%m-%d %H:%M:%S")): "ENTRANCE OVER MAX"})
        return CODE_FULL
    return CODE_NOT_FULL

def updateMax(max):
    if checkInt(max):
        db.update({'max': max})
        return MAX_UPDATED
    return MAX_UPDATE_FAILURE


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
