import socket, time, json
from flask import Flask, request,jsonify

# Dirección IP y puerto del servidor
SERVER_IP = '192.168.0.102'  # Cambia esta dirección IP por la de tu servidor
SERVER_PORT = 8765


app = Flask(__name__)

def send_message(sock, message):
    sock.sendall(message.encode())
    response = sock.recv(1024)
    return response.decode()



@app.route('/embebidos/outputs', methods=['POST'])
def set_outputs():
    #global outputs
    data = json.loads(request.data.decode())
    outputs = data["outputs"]
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((SERVER_IP, SERVER_PORT))
    response = send_message(client_socket, outputs)
    print('Recibido', outputs)
    client_socket.close()
    return "OK"


@app.route('/embebidos/inputs', methods=['GET'])
def get_inputs():
    
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((SERVER_IP, SERVER_PORT))
    response = send_message(client_socket, 'get_inputs')
    print('Estados de las entradas GPIO:', response)
    client_socket.close()
    #time.sleep(0.1)
    return response

def receive_chunks(socket):
    chunks = []
    
    # Receive the first chunk
    #chunk = socket.recv(1024)
    #if not chunk:
    #    return b''
    #chunks.append(chunk)
    
    # Receive subsequent chunks
    #first = False

    while True:
        chunk = socket.recv(1024)
        if not chunk:
            break
        chunks.append(chunk)
        
    return b''.join(chunks)
    #return chunk



@app.route('/embebidos/image', methods=['GET'])
def get_image():
    
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((SERVER_IP, SERVER_PORT))
    
    with client_socket:
        response = send_message(client_socket, 'get_images')
        image_data = receive_chunks(client_socket)
        print('Image data:', image_data)
    
        client_socket.close()

    return image_data

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8777)



