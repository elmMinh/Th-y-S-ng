import socket

print('Creating socket...')
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print('Socket created')

# Kết nối với remote host
target_host = "www.google.com"
target_port = 80
print("Connecting with remote host...")
s.connect((target_host, target_port))
print('Connection OK')

# Gửi HTTP request
request = "GET / HTTP/1.1\r\nHost: %s\r\n\r\n" % target_host
s.send(request.encode())

# Nhận phản hồi từ server
data = s.recv(4096)
print("Data:", data.decode())  # Chuyển dữ liệu nhận được từ bytes sang string
print("Length:", len(data))

# Đóng socket
print('Closing the socket...')
s.close()
