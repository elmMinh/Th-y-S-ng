import socket

# Tạo socket TCP
mySocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
mySocket.bind(('0.0.0.0', 8080))  # Lắng nghe trên tất cả các địa chỉ IP
mySocket.listen(5)  # Tối đa 5 kết nối chờ

print("HTTP Server is running on port 8080...")

while True:
    print("Waiting for connections...")
    recvSocket, address = mySocket.accept()  # Chờ client kết nối
    print(f"HTTP request received from {address}:")
    request = recvSocket.recv(1024).decode()
    print(request)  # In request từ client

    # Trả về phản hồi HTTP 200 OK
    response = "HTTP/1.1 200 OK\r\n\r\n<html><body><h1>Hello World!</h1></body></html>\r\n"
    recvSocket.send(response.encode('utf-8'))
    recvSocket.close()
