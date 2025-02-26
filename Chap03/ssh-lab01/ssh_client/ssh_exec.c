#include <libssh/libssh.h>
#include <stdio.h>
#include <stdlib.h>

void execute_remote_command(ssh_session session, const char *command) {
    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;

    channel = ssh_channel_new(session);
    if (channel == NULL) {
        fprintf(stderr, "Error creating channel\n");
        return;
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error opening channel: %s\n", ssh_get_error(session));
        ssh_channel_free(channel);
        return;
    }

    rc = ssh_channel_request_exec(channel, command);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error executing command: %s\n", ssh_get_error(session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return;
    }

    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, nbytes, stdout);
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

int main() {
    ssh_session my_ssh_session;
    int rc;
    
    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL) {
        fprintf(stderr, "Error creating SSH session\n");
        return -1;
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "ssh-server");  // Change to server IP if needed
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &(int){22});
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "seed");

    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting: %s\n", ssh_get_error(my_ssh_session));
        ssh_free(my_ssh_session);
        return -1;
    }

    rc = ssh_userauth_password(my_ssh_session, NULL, "dees");  // Replace with correct password
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(my_ssh_session));
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    printf("Successfully connected and authenticated!\n");

    // Execute a command remotely
    execute_remote_command(my_ssh_session, "ls -l");  // Example command

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);

    return 0;
}
