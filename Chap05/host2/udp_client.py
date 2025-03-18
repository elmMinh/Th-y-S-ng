import socket

SERVER_IP = "host1"  # Địa chỉ của server trong Docker network
SERVER_PORT = 6789

# Tạo socket UDP
client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    message = input("Enter your message > ")
    if message.lower() == "quit":
        break

    # Gửi tin nhắn đến server
    client_socket.sendto(message.encode(), (SERVER_IP, SERVER_PORT))

    # Nhận phản hồi từ server
    response, _ = client_socket.recvfrom(4096)
    print("Server response:", response.decode())

client_socket.close()
