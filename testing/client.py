import socket

HOST = "127.0.0.1"
PORT = 8080

def connectToServer():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        print(f"Connecting to {HOST}:{PORT}...")
        s.connect((HOST, PORT))
        print(f"Connected!")

        data = s.recv(1024)  # buffer size 1024 bytes
        if data:
            print(f"Received from server: {data.decode('utf-8')}")
        else:
            print("No data received.")

if __name__ == "__main__":
    connectToServer()