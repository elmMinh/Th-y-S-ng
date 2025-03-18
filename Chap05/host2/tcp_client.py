import socket

SERVER_IP = "host1"  # Địa chỉ server (host1 trong Docker network)
SERVER_PORT = 9998

try:
    # Tạo socket TCP
    mysocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    mysocket.connect((SERVER_IP, SERVER_PORT))
    print(f"Connected to {SERVER_IP} on port {SERVER_PORT}")

    # Nhận thông báo từ server
    message = mysocket.recv(1024).decode()
    print("Message from server:", message)

    while True:
        message = input("Enter your message > ")
        mysocket.send(message.encode())
        if message.lower() == "quit":
            break
        response = mysocket.recv(1024).decode()
        print("Server response:", response)

except socket.error as error:
    print("Socket error:", error)
finally:
    mysocket.close()
