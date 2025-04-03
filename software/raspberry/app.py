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
    

#INIT
if __name__ == '__main__':
    socketio.run(app, host="0.0.0", port=8000, debug=True)