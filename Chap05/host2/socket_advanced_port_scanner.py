import optparse
from socket import *
from threading import Thread

def socketScan(host, port):
    """ Kiểm tra xem cổng có mở không """
    try:
        socket_connect = socket(AF_INET, SOCK_STREAM)
        socket_connect.settimeout(1)
        socket_connect.connect((host, port))
        print(f"[+] {port}/tcp OPEN")
    except Exception as exception:
        print(f"[-] {port}/tcp CLOSED - {exception}")
    finally:
        socket_connect.close()

def portScanning(host, ports):
    """ Thực hiện quét cổng bằng nhiều luồng """
    try:
        ip = gethostbyname(host)
        print(f"[+] Scanning {host} ({ip})...")
    except:
        print(f"[-] Cannot resolve '{host}': Unknown host")
        return
    
    for port in ports:
        t = Thread(target=socketScan, args=(ip, int(port)))
        t.start()

def main():
    """ Nhận tham số dòng lệnh và khởi động quét """
    parser = optparse.OptionParser("Usage: socket_advanced_port_scanner.py -H <Host> -P <Port>")
    parser.add_option('-H', dest='host', type='string', help='Specify target host')
    parser.add_option('-P', dest='port', type='string', help='Specify ports (comma separated)')
    
    (options, args) = parser.parse_args()
    
    if not options.host or not options.port:
        print(parser.usage)
        exit(0)
    
    ports = options.port.split(',')
    portScanning(options.host, ports)

if __name__ == '__main__':
    main()
