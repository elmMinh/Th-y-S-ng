#include <libssh/libssh.h>
#include <stdio.h>   // Thêm thư viện cần thiết
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LOCAL_PORT 9000
#define REMOTE_HOST "web_server"  // Chuyển tiếp đến container web server
#define REMOTE_PORT 8080

void handle_connection(int client_sock, ssh_session session) {
    ssh_channel channel = ssh_channel_new(session);
    if (channel == NULL) {
        fprintf(stderr, "Error creating channel: %s\n", ssh_get_error(session));
        close(client_sock);
        return;
    }

    int rc = ssh_channel_open_forward(channel, REMOTE_HOST, REMOTE_PORT, "0.0.0.0", LOCAL_PORT);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error opening forward channel: %s\n", ssh_get_error(session));
        ssh_channel_free(channel);
        close(client_sock);
        return;
    }

    char buffer[1024];
    int nbytes;
    while ((nbytes = read(client_sock, buffer, sizeof(buffer))) > 0) {
        ssh_channel_write(channel, buffer, nbytes);
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
        if (nbytes > 0) write(client_sock, buffer, nbytes);
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    close(client_sock);
}

int main() {
    const char *ssh_host = "172.19.0.2";  // Dùng tên container SSH server

    ssh_session session = ssh_new();
    if (!session) {
        fprintf(stderr, "Error creating SSH session\n");
        exit(-1);
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, ssh_host);
    
    int port = 22;  // Sửa lỗi kiểu dữ liệu
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    
    ssh_options_set(session, SSH_OPTIONS_USER, "seed");

    int rc = ssh_connect(session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting: %s\n", ssh_get_error(session));
        ssh_free(session);
        exit(-1);
    }

    rc = ssh_userauth_password(session, NULL, "dees");
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Error authenticating: %s\n", ssh_get_error(session));
        ssh_disconnect(session);
        ssh_free(session);
        exit(-1);
    }

    printf("Successfully connected and authenticated!\n");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        ssh_disconnect(session);
        ssh_free(session);
        exit(-1);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(LOCAL_PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(sockfd);
        ssh_disconnect(session);
        ssh_free(session);
        exit(-1);
    }

    if (listen(sockfd, 5) < 0) {
        perror("Error listening on socket");
        close(sockfd);
        ssh_disconnect(session);
        ssh_free(session);
        exit(-1);
    }

    printf("Listening on localhost:%d, forwarding to %s:%d\n", LOCAL_PORT, REMOTE_HOST, REMOTE_PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Error accepting connection");
            continue;
        }

        handle_connection(client_sock, session);
    }

    close(sockfd);
    ssh_disconnect(session);
    ssh_free(session);
    return 0;
}
