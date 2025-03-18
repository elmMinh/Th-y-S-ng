#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // Để sử dụng sleep()

int main() {
    ssh_session my_ssh_session;
    ssh_channel channel;
    int rc;
    int verbosity = SSH_LOG_PROTOCOL;
    int local_port = 9000;  // Port trên host machine
    int remote_port = 8080; // Port trên web server container
    const char *remote_host = "web_server"; // Tên container web-server trong docker network

    // Khởi tạo SSH session
    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL) {
        fprintf(stderr, "Error creating SSH session\n");
        return -1;
    }

    // Thiết lập các tùy chọn cho SSH session
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "ssh_server"); // Kết nối qua ssh-server
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "user"); // Thay bằng username của bạn nếu cần
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

    // Kết nối đến SSH server
    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting to SSH server: %s\n", ssh_get_error(my_ssh_session));
        ssh_free(my_ssh_session);
        return -1;
    }

    // Xác thực bằng password (có thể thay bằng key-based)
    rc = ssh_userauth_password(my_ssh_session, NULL, "password"); // Thay "password" bằng mật khẩu thực tế
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Error authenticating: %s\n", ssh_get_error(my_ssh_session));
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    printf("Successfully connected and authenticated!\n");

    // Tạo kênh cho port forwarding
    channel = ssh_channel_new(my_ssh_session);
    if (channel == NULL) {
        fprintf(stderr, "Error creating channel\n");
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    // Thiết lập local port forwarding (từ localhost:9000 đến web_server:8080)
    rc = ssh_channel_open_forward(channel, remote_host, remote_port, "0.0.0.0", local_port);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error opening forward channel: %s\n", ssh_get_error(my_ssh_session));
        ssh_channel_free(channel);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    printf("Port forwarding established: localhost:%d -> %s:%d\n", local_port, remote_host, remote_port);
    printf("Access http://localhost:%d to reach the web server.\n", local_port);

    // Giữ chương trình chạy để duy trì kết nối
    while (1) {
        sleep(1); // Có thể thêm logic xử lý dữ liệu nếu cần
    }

    // Giải phóng tài nguyên (không chạy đến đây vì vòng lặp vô hạn)
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);

    return 0;
}