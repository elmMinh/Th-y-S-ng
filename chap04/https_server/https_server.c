#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 9443
#define CERT_FILE "cert.pem"
#define KEY_FILE "key.pem"
#define BUFFER_SIZE 1024

// Khởi tạo OpenSSL context
SSL_CTX* init_ssl_context() {
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        perror("SSL_CTX_new failed");
        exit(EXIT_FAILURE);
    }

    // Load chứng chỉ và khóa riêng
    if (SSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, KEY_FILE, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// Xử lý kết nối của client
void handle_client(SSL *ssl) {
    char buffer[BUFFER_SIZE] = {0};
    
    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }

    SSL_read(ssl, buffer, sizeof(buffer));
    printf("Received:\n%s\n", buffer);

    const char *response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, cả nhà yêu của chồng!\n";
    
    SSL_write(ssl, response, strlen(response));
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

int main() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    SSL_CTX *ctx = init_ssl_context();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("HTTPS Server running on port %d...\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("Accept failed");
            continue;
        }

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);

        handle_client(ssl);
        close(client_fd);
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    return 0;
}
