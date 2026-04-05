import socket
import json
import time

HOST = "127.0.0.1"
PORT = 8080

MAX_RETRIES = 10
RETRY_DELAY_IN_SECS = 2

def buildHttpPostRequest(paymentData: dict) -> bytes:
    body = json.dumps(paymentData)
    request = (
        f"POST / HTTP/1.1\r\n"
        f"Host: {HOST}:{PORT}\r\n"
        f"Content-Type: application/json\r\n"
        f"Content-Length: {len(body)}\r\n"
        f"Connection: close\r\n"
        f"\r\n"
        f"{body}"
    )
    return request.encode('utf-8')

def connectToServer():
    retries = 0
    while retries < MAX_RETRIES:
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                print(f"Connecting to {HOST}:{PORT}... (attempt {retries + 1}/{MAX_RETRIES})")
                s.connect((HOST, PORT))
                print("Connected!")

                paymentData = {
                    "amount_cents": 1234,
                    "currency": "AUD",
                    "card_number": "1234567890",
                    "cvc": "123"
                }
                request = buildHttpPostRequest(paymentData)
                s.sendall(request)
                print(f"Sent payment request:\n{json.dumps(paymentData, indent=2)}")

                data = s.recv(1024)
                if data:
                    print(f"Received from server: {data.decode('utf-8')}")
                else:
                    print("No data received.")
                return

        except ConnectionRefusedError:
            retries += 1
            if retries >= MAX_RETRIES:
                print(f"Server not available after {MAX_RETRIES} attempts. Giving up.")
                return
            print(f"Server not running. Retrying in {RETRY_DELAY_IN_SECS}s...")
            time.sleep(RETRY_DELAY_IN_SECS)

if __name__ == "__main__":
    connectToServer()