from flask import Flask, render_template
from flask_socketio import SocketIO, emit

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'  

socketio = SocketIO(app)

if __name__ == '__main__':
    socketio.run(app)

laps = 0

@app.route("/")
def index():
    return render_template('index.html', laps=laps)

@app.route("/getLaps")
def getLaps():
    global laps
    laps += 1
    socketio.emit('update_laps', {'laps': laps})  
    return str(laps)

@socketio.on('connect')
def handle_connect():
    emit('update_laps', {'laps': laps}) 

if __name__ == '__main__':
    socketio.run(app) 