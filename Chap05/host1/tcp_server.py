import socket
import threading

SERVER_IP = "0.0.0.0"  # Lắng nghe trên tất cả các địa chỉ
SERVER_PORT = 9998     # Cổng của server

# Tạo socket TCP
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((SERVER_IP, SERVER_PORT))
server.listen(5)

print(f"[*] Server listening on {SERVER_IP}:{SERVER_PORT}")

def handle_client(client_socket, addr):
    print(f"[+] Accepted connection from {addr[0]}:{addr[1]}")
    
    client_socket.send("I am the server accepting connections...".encode())

    while True:
        try:
            request = client_socket.recv(1024).decode()
            if not request or request.lower() == "quit":
                print(f"[-] Connection closed by {addr[0]}:{addr[1]}")
                break
            print(f"[*] Received from {addr}: {request}")
            client_socket.send("ACK".encode())
        except ConnectionResetError:
            print(f"[!] Connection lost from {addr}")
            break
    
    client_socket.close()

while True:
    client, addr = server.accept()
    client_thread = threading.Thread(target=handle_client, args=(client, addr))
    client_thread.start()
