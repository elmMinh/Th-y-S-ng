import socket
import threading

HOST = '0.0.0.0'  # Lắng nghe trên tất cả IP
PORT = 8080       # Cổng server
MAX_CONN = 5      # Số kết nối tối đa

# Tạo socket TCP
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(MAX_CONN)

print(f"Server is listening on {HOST}:{PORT}...")

def handle_client(client_socket, address):
    print(f"[+] New connection from {address}")

    # Nhận dữ liệu từ client
    data = client_socket.recv(1024).decode()
    print(f"Received from {address}: {data}")

    # Gửi phản hồi
    response = f"Hello {address}, you said: {data}"
    client_socket.send(response.encode())

    # Đóng kết nối
    client_socket.close()
    print(f"[-] Connection closed for {address}")

while True:
    client_socket, addr = server.accept()
    client_thread = threading.Thread(target=handle_client, args=(client_socket, addr))
    client_thread.start()
