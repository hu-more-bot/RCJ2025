from flask import Flask, render_template
from flask_socketio import SocketIO, emit

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'  

socketio = SocketIO(app)


laps = 0
race_going = False


def addToLaps():
    laps += 1
    socketio.emit("update_laps", {"laps":laps})


# ROUTES
@app.route("/")
def index():
    return render_template('index.html', laps=laps)

@app.route("/dev")
def dev():
    return render_template('dev.html')

#WEBSOCKET
@socketio.on('connect')
def handle_connect():
    emit('update_laps', {'laps': laps}) 

@socketio.on("start")
def start_race():
    race_going = True
    
@socketio.on("end")
def end_race():
    race_going = False

#INIT
if __name__ == '__main__':
    socketio.run(app, host="0.0.0", port=8000, debug=True)