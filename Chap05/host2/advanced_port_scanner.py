import socket

TARGET_IP = "host1"  # Địa chỉ của server trong Docker network
PORTS = [22, 80, 443, 8080, 9000]  # Các cổng cần quét

def scan_port(ip, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(1)  # Timeout sau 1 giây
    result = s.connect_ex((ip, port))  # Kết nối kiểm tra cổng

    if result == 0:
        print(f"[+] Port {port} is OPEN")
        s.send(f"Hello from scanner to {port}".encode())
        response = s.recv(1024).decode()
        print(f"Response: {response}")
    else:
        print(f"[-] Port {port} is CLOSED")

    s.close()

print(f"Scanning {TARGET_IP}...")
for port in PORTS:
    scan_port(TARGET_IP, port)
