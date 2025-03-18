import socket
import threading

TARGET_IP = "host1"  # Địa chỉ server
PORT = 8080

def connect_to_server(client_id):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((TARGET_IP, PORT))

    message = f"Hello các con vợ của anh from J97 {client_id}"
    s.send(message.encode())

    response = s.recv(1024).decode()
    print(f"[Client {client_id}] Server response: {response}")

    s.close()

# Tạo và chạy nhiều client song song
threads = []
for i in range(5):  # 5 client kết nối cùng lúc
    t = threading.Thread(target=connect_to_server, args=(i,))
    t.start()
    threads.append(t)

# Đợi tất cả client hoàn thành
for t in threads:
    t.join()
