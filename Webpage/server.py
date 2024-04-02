from flask import Flask,render_template,request,redirect,url_for,jsonify
from waitress import serve
import requests
import base64
from PIL import Image
from io import BytesIO

app=Flask(__name__,static_url_path='/static',static_folder='static')


users={'admin':'admin'}
button_states = {'Light1': False, 'Light2': False, 'Light3': False,'Light4': False,'Light5': False}
door_states={'Door1':False,'Door2':False,'Door3':False,'Door4':False}
showpic=False

def send_request_to_pi(endpoint, data=None):
    try:
        response = requests.post(f"http://127.0.0.1:8777/embebidos/{endpoint}", json=data)
        response.raise_for_status()
        return "OK"
    except requests.exceptions.RequestException as e:
        # Handle request errors
        print(f"Request error: {e}")
        return None

@app.route('/index')
def home():
    return render_template('home.html',button_states=button_states,door_states=door_states,showpic=showpic)

@app.route('/Img')
def image():
    return render_template('image.html',showpic=showpic)


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

@app.route('/get_output',methods=['GET'])
def get_output_data():
    try:
        response=requests.get('http://127.0.0.1:8777/embebidos/inputs')
        response.raise_for_status()
        data=response.text
        i=0
        data = str(data)
        #door_array = ["Door1","Door2","Door3","Door4"]
        for key in door_states.keys():
            if(data[i]=='0'):
                door_states[key]=False
            else:
                door_states[key]=True
            i+=1
        return redirect('/index')
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
    send_request_to_pi("outputs", data=data)

    # Redirect back to the home page
    return redirect('/index')

@app.route('/toggle_all', methods=['POST'])
def toggle_all():
    global button_states
    
    # Send request to Raspberry Pi API
    outputs=""
    for key in button_states.keys():
        if button_states[key]==False:
            outputs+="1"
            button_states[key]=True
        else:
            outputs+="0"
            button_states[key]=False

    data = {"outputs":outputs}
    send_request_to_pi("outputs", data=data)

    # Redirect back to the home page
    return redirect('/index')

@app.route('/takeimg',methods=['GET'])
def Take_Img():
    try:
        response=requests.get('http://127.0.0.1:8777/embebidos/image')
        response.raise_for_status()
        image_data=response.text
        base64_to_png(image_data, 'static/Image/image.png')
        global showpic
        showpic=True
        return redirect('/Img')
    
    except requests.exceptions.RequestException as e:
        print(f"Error processing data: {e}")
        return e


def base64_to_png(base64_string, output_filename='output.png'):
    try:
        # Decode the base64 string
        image_data = base64.b64decode(base64_string)
        
        # Write the binary data to a PNG file
        with open(output_filename, 'wb') as f:
            f.write(image_data)
        
        print(f"Successfully converted and saved as '{output_filename}'")
    except Exception as e:
        print("Error:", e)



if __name__=="__main__":
    serve(app,host="0.0.0.0",port=5000)
