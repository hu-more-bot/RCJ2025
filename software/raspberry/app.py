from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import time

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'  

socketio = SocketIO(app)


laps = 0
race_start = 0 # miliseconds unix time since start
race_end = 0
lap_times = [] 


running = True

def updateData():
    socketio.emit("update_data", {"laps" : laps, "lap_times" : lap_times, "race_end" : race_end, "race_start" : race_start})
    print("===== UPDATED DATA =====")

def getMillis():
    return round(time.time() * 1000)

def addToLaps():
    global laps, lap_times
    if(race_start > 0 and race_end == 0):
        laps += 1
        lap_times.append(getMillis())
    updateData()

# ROUTES
@app.route("/")
def index():
    global laps, lap_times, race_end, race_start
    return render_template('index.html', laps=laps, race_start=race_start, race_end=race_start,  lap_times=lap_times)

@app.route("/dev")
def dev():
    global laps, lap_times, race_end, race_start
    return render_template('dev.html', laps=laps, race_start=race_start, race_end=race_end,  lap_times=lap_times)

#WEBSOCKET
@socketio.on('connect')
def handle_connect():
    global laps, lap_times, race_end, race_start
    emit('update_data', {"laps" : laps, "lap_times" : lap_times, "race_end" : race_end, "race_start" : race_start})

@socketio.on("start")
def start_race():
    global race_start, race_end, lap_times, laps
    if((race_start == 0 and race_end == 0)or(race_start > 0 and race_end>0)):
        race_start = getMillis()
        race_end = 0
        lap_times = []
        laps=0
        updateData()
        
    
@socketio.on("end")
def end_race():
    global race_start, race_end
    if(race_start > 0 and race_end == 0):
        race_end= getMillis()
        updateData()
    
    
@socketio.on("DEV_ADD")
def DEVADD():
    addToLaps()
    


    
# ====================== GPIO STUFF ==============

import RPi.GPIO as GPIO
import threading

# GPIO Pin Setup
LASER_GPIO = 17  # Pin for the Laser module (optional, if controlling laser power)
RECEIVER_GPIO = 4  # Pin for the Laser receiving module
ARDUINO_GPIO = 26

# Initialize GPIO
GPIO.setmode(GPIO.BCM)

# in pin
GPIO.setup(RECEIVER_GPIO, GPIO.IN)

# out pins
GPIO.setup(LASER_GPIO, GPIO.OUT)
GPIO.output(LASER_GPIO, GPIO.HIGH)  # Turn on the laser

GPIO.setup(ARDUINO_GPIO, GPIO.OUT)
GPIO.output(ARDUINO_GPIO, GPIO.LOW) # dont send data to arduino, plug in arduino after running this program

beam_was_unbroken = True  # Track beam state
def MonitorLaser():
    global beam_was_unbroken, running  
    while running:
        print(GPIO.input(RECEIVER_GPIO))
        
        # Check if laser beam is interrupted
        if GPIO.input(RECEIVER_GPIO) == GPIO.LOW and beam_was_unbroken:
            addToLaps()
            beam_was_unbroken = False  # Mark that the beam was broken

            time.sleep(0.5)  # Debounce
        elif GPIO.input(RECEIVER_GPIO) == GPIO.HIGH:
            beam_was_unbroken = True  # Reset when beam is restored

        time.sleep(0.2)

#INIT
if __name__ == '__main__':
    try:
        # Start laser monitoring thread
        monitor_thread = threading.Thread(target=MonitorLaser)
        monitor_thread.daemon = True  # Daemonize thread
        monitor_thread.start()
        
        print("Starting server with laser monitoring...")
        socketio.run(app, host="0.0.0.0", port=8000, debug=True)
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        running = False  # Stop the monitoring thread
        monitor_thread.join()  # Wait for thread to finish
        GPIO.cleanup()
        print("Cleanup complete")
    
    