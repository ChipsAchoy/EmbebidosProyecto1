from flask import Flask,render_template,request,redirect,url_for
from waitress import serve
from apscheduler.schedulers.background import BackgroundScheduler
import requests

app=Flask(__name__,static_url_path='/static',static_folder='static')

scheduler=BackgroundScheduler(daemon=True)
scheduler.start()

users={'admin':'admin'}
button_states = {'Light1': False, 'Light2': False, 'Light3': False,'Light4': False,'Light5': False}
door_states={'Door1':False,'Door2':False,'Door3':False,'Door4':False}

def send_request_to_pi(endpoint, data=None):
    try:
        response = requests.post(f"http://127.0.0.1/embebidos/{endpoint}", json=data)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        # Handle request errors
        print(f"Request error: {e}")
        return None

@app.route('/index')
def home():
    return render_template('home.html',button_states=button_states,door_states=door_states)

@app.route('/')
@app.route('/login',methods=['GET','POST'])
def login():
    if request.method=="POST":
        username=request.form['username']
        password=request.form['password']
        if username in users and users[username]==password:
            return redirect(url_for('home'))
        else:
            error="Invalid username or password. Please try again"
            return  render_template('login.html',error=error)
    return render_template('login.html')

def get_output_data():
    try:
        response=requests.get('https://127.0.0.1/embebidos/outputs')
        response.raise_for_status()
        data=response.json()
        print(response)
        i=0
        for key in door_states.keys():
            if(data[i]==0):
                door_states[key]=False
            else:
                door_states[key]=True
            i+=1

        return data, 200
    except requests.exceptions.RequestException as e:
        print(f"Error processing data: {e}")

@app.route('/toggle_button/<button_id>', methods=['POST'])
def toggle_button(button_id):
    global button_states
    # Toggle the state of the button
    button_states[button_id] = not button_states[button_id]

    # Send request to Raspberry Pi API
    outputs=""
    for key in button_states.keys():
        if button_states[key]==False:
            outputs+="0"
        else:
            outputs+="1"
    data = {"outputs":outputs}
    print(data)
    send_request_to_pi("inputs", data=data)

    # Redirect back to the home page
    return redirect('/index')


scheduler.add_job(get_output_data, 'interval', seconds=10)

if __name__=="__main__":
    serve(app,host="0.0.0.0",port=5000)