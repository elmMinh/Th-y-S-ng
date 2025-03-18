import socket

webhost = 'host1'  # Kết nối đến server trong Docker network
webport = 8080

print("Contacting %s on port %d ..." % (webhost, webport))

# Tạo socket TCP client
webclient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
webclient.connect((webhost, webport))

# Gửi HTTP GET request
request = "GET / HTTP/1.1\r\nHost: host1\r\nConnection: close\r\n\r\n"
webclient.send(request.encode('utf-8'))

# Nhận phản hồi từ server
reply = webclient.recv(4096)

print("Response from %s:" % webhost)
print(reply.decode())

# Đóng kết nối
webclient.close()
