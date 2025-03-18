import socket

SERVER_IP = "0.0.0.0"  # Lắng nghe trên tất cả các địa chỉ
SERVER_PORT = 6789     # Cổng của server

# Tạo socket UDP
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_socket.bind((SERVER_IP, SERVER_PORT))

print(f"[*] UDP Server listening on {SERVER_IP}:{SERVER_PORT}")

while True:
    # Nhận dữ liệu từ client
    data, addr = server_socket.recvfrom(4096)
    print(f"[+] Message from {addr}: {data.decode()}")

    # Phản hồi client
    response = "Hello from UDP server!"
    server_socket.sendto(response.encode(), addr)
