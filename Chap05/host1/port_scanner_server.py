import socket

HOST = '0.0.0.0'  # Lắng nghe trên tất cả các IP
PORT = 8080       # Cổng server
MAX_CONN = 5      # Số kết nối tối đa

# Tạo socket TCP
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(MAX_CONN)

print(f"Server is listening on {HOST}:{PORT}...")

while True:
    client, addr = server.accept()  # Chấp nhận kết nối
    print(f"Accepted connection from {addr}")

    # Nhận dữ liệu từ client
    data = client.recv(1024).decode()
    print(f"Received: {data}")

    # Phản hồi client
    client.send("ACK".encode())

    client.close()
