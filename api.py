from time import sleep
from threading import Thread
import requests
from bs4 import BeautifulSoup
from flask import *
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.support.ui import WebDriverWait
import paho.mqtt.client as mqtt_client
from requests import get
from re import match

app = Flask(__name__)

@app.route("/panneau-solaire/consommation/actuelle")
def consommation_actuelle():
    url = "http://192.168.0.2/index.php"

    response = requests.get(url)

    if response.ok:
        soup = BeautifulSoup(response.content, 'html5lib')
        title = soup.findAll("td")

        temperature = ""
        for i in title[2].text:
            try:
                number = int(i)
                temperature += i
            except:
                pass
        return temperature

@app.route("/panneau-solaire/consommation/journaliere")
def consommation_journaliere():
    url = "http://192.168.0.2/index.php"

    response = requests.get(url)

    if response.ok:
        soup = BeautifulSoup(response.content, 'html5lib')
        title = soup.findAll("td")

        chaine = ""
        for i in title[3].text:
            try:
                number = int(i)
                chaine += i
            except:
                if(i == "." or i == ","):
                    chaine += i
        consommation = float(chaine)
        consommation *= 1000
        return str(consommation)

@app.route("/panneau-solaire/temperature")
def index():
    url = "http://192.168.0.2/index.php/realtimedata"

    response = requests.get(url)

    if response.ok:
        soup = BeautifulSoup(response.content, 'html5lib')
        title = soup.findAll("td")

        temperature = ""
        for i in title[4].text:
            try:
                number = int(i)
                temperature += i
            except:
                pass
        return temperature

@app.route("/consommation/1")
def consommation1():
    url = "http://192.168.0.2/index.php/realtimedata"

    response = requests.get(url)

    if response.ok:
        soup = BeautifulSoup(response.content, 'html5lib')
        title = soup.findAll("td")

        temperature = ""
        for i in title[1].text:
            try:
                number = int(i)
                temperature += i
            except:
                pass
        return temperature

@app.route("/consommation")
def consommation():
    url = "http://192.168.0.2/index.php/realtimedata/energy_graph"

    response = requests.get(url)

    if response.ok:
        soup = BeautifulSoup(response.content, 'html5lib')
        title = soup.findAll("script")
        js = title[6]
        js = str(js)
        chaine = "data: ["
        while True:
            if(js.startswith(chaine)):
                break
            else:
                js = js[1:]
        
        aEffacer = False
        newChaine = ""
        for i in range(len(js)):
            caractere = js[i]
            if(i > len(chaine) - 1):
                newChaine += caractere
        js = newChaine
        newChaine = ""
        for i in range(len(js)):
            caractere = js[i]
            if(caractere== "]"):
                aEffacer = True
            if(aEffacer == False and caractere != " "):
                newChaine += caractere
        
        newChaine.replace("[", "")
        liste = list(newChaine.split(","))
        if(liste[-1] == ""):
            return liste[-2]
        
        return liste[-1]

def on_message(client, userdata, message):
	topic = message.topic
	data = str(message.payload.decode("utf-8"))
	
	if match("^ipxv3/action-sortie-[1-8]$", topic):
		if data == "command":
			get("http://192.168.0.3/leds.cgi?led=" + str(int(topic[-1])-1))
		elif data == "on":
			get("http://192.168.0.3/preset.htm?set" + topic[-1] + "=1")
		elif data == "off":
			get("http://192.168.0.3/preset.htm?set" + topic[-1] + "=0")

client = mqtt_client.Client()
client.connect(host="127.0.0.1",port=1883,keepalive=45)
client.subscribe("ipxv3/#")
client.on_message = on_message
client.loop_start()

options = webdriver.ChromeOptions()
options.add_argument('headless')
options.add_argument('--no-sandbox')
driver = webdriver.Chrome("/usr/lib/chromium-browser/chromedriver",options=options)
driver.get("http://192.168.0.3/index1.htm")
sleep(2)
def ipxv3():
	etats_sorties_ipxv3 = [False for _ in range(8)]
	while True:
		for i in range(8):
			element = driver.find_element_by_id("led"+str(i))
			if element.get_attribute("class") == "loff":
				etats_sorties_ipxv3[i] = False
				client.publish("ipxv3/etat-sortie-" + str(i+1), "0")
			else:
				etats_sorties_ipxv3[i] = True
				client.publish("ipxv3/etat-sortie-"+str(i+1),"1")

			
ipxv3_thread = Thread(target=ipxv3)
ipxv3_thread.start()

app.run("0.0.0.0","8100")
