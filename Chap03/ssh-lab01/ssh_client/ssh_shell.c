#include <libssh/libssh.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void error_exit(ssh_session session, const char *msg) {
    fprintf(stderr, "%s: %s\n", msg, ssh_get_error(session));
    ssh_disconnect(session);
    ssh_free(session);
    exit(EXIT_FAILURE);
}

int main() {
    ssh_session session;
    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;

    // Khởi tạo session SSH
    session = ssh_new();
    if (session == NULL) {
        fprintf(stderr, "Failed to create SSH session\n");
        return EXIT_FAILURE;
    }

    // Thiết lập thông tin SSH server
    ssh_options_set(session, SSH_OPTIONS_HOST, "ssh-server");
    ssh_options_set(session, SSH_OPTIONS_PORT, &(int){22});
    ssh_options_set(session, SSH_OPTIONS_USER, "seed");

    // Kết nối SSH
    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        error_exit(session, "Error connecting");
    }

    // Xác thực bằng mật khẩu
    rc = ssh_userauth_password(session, NULL, "dees");
    if (rc != SSH_AUTH_SUCCESS) {
        error_exit(session, "Authentication failed");
    }

    printf("Successfully connected and authenticated!\n");

    // Mở kênh giao tiếp SSH
    channel = ssh_channel_new(session);
    if (channel == NULL) {
        error_exit(session, "Failed to create SSH channel");
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        error_exit(session, "Error opening SSH channel");
    }

    // Yêu cầu pseudo-terminal
    rc = ssh_channel_request_pty(channel);
    if (rc != SSH_OK) {
        error_exit(session, "Error requesting PTY");
    }

    // Yêu cầu shell
    rc = ssh_channel_request_shell(channel);
    if (rc != SSH_OK) {
        error_exit(session, "Error requesting shell");
    }

    // Thiết lập non-blocking I/O
    ssh_channel_set_blocking(channel, 0);

    printf("Interactive shell started. Type commands or 'exit' to quit.\n");

    // Vòng lặp đọc/ghi dữ liệu từ shell
    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds); // STDIN
        FD_SET(ssh_get_fd(session), &fds);

        select(ssh_get_fd(session) + 1, &fds, NULL, NULL, NULL);

        // Đọc từ STDIN và gửi đến SSH
        if (FD_ISSET(0, &fds)) {
            int bytes = read(0, buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                ssh_channel_write(channel, buffer, bytes);
            }
        }

        // Đọc từ SSH và in ra màn hình
        if (FD_ISSET(ssh_get_fd(session), &fds)) {
            nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0);
            if (nbytes > 0) {
                buffer[nbytes] = '\0';
                printf("%s", buffer);
                fflush(stdout);
            } else if (nbytes == 0) {
                break; // Kết thúc khi SSH đóng kết nối
            }
        }
    }

    // Đóng channel khi kết thúc
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    // Ngắt kết nối SSH
    ssh_disconnect(session);
    ssh_free(session);

    return 0;
}
