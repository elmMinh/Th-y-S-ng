#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER_IP "172.21.0.2" // Đổi nếu server chạy trên máy khác
#define SERVER_PORT 9443
#define REQUEST "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"

SSL_CTX* init_ssl_context() {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        perror("SSL_CTX_new failed");
        exit(EXIT_FAILURE);
    }
    return ctx;
}

int create_socket(const char *ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void https_request(SSL *ssl) {
    SSL_write(ssl, REQUEST, strlen(REQUEST));

    char buffer[4096] = {0};
    int bytes;
    while ((bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes] = '\0';
        printf("%s", buffer);
    }
}

int main() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = init_ssl_context();

    int sockfd = create_socket(SERVER_IP, SERVER_PORT);
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);

    if (SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    printf("Connected to HTTPS Server!\n");
    https_request(ssl);

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);

    return 0;
}
