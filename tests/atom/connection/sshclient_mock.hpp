#ifndef SSHCLIENT_MOCK_HPP
#define SSHCLIENT_MOCK_HPP

#include <gmock/gmock.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

class MockSSHSession {
public:
    MOCK_METHOD(ssh_session, ssh_new, (), ());
    MOCK_METHOD(int, ssh_options_set, (ssh_session, int, const void*), ());
    MOCK_METHOD(int, ssh_connect, (ssh_session), ());
    MOCK_METHOD(void, ssh_disconnect, (ssh_session), ());
    MOCK_METHOD(void, ssh_free, (ssh_session), ());
    MOCK_METHOD(int, ssh_userauth_password, (ssh_session, const char*, const char*), ());
    MOCK_METHOD(ssh_channel, ssh_channel_new, (ssh_session), ());
    MOCK_METHOD(int, ssh_channel_open_session, (ssh_channel), ());
    MOCK_METHOD(int, ssh_channel_request_exec, (ssh_channel, const char*), ());
    MOCK_METHOD(int, ssh_channel_read, (ssh_channel, void*, uint32_t, int), ());
    MOCK_METHOD(void, ssh_channel_send_eof, (ssh_channel), ());
    MOCK_METHOD(void, ssh_channel_close, (ssh_channel), ());
    MOCK_METHOD(void, ssh_channel_free, (ssh_channel), ());
    MOCK_METHOD(const char*, ssh_get_error, (ssh_session), ());

    MOCK_METHOD(sftp_session, sftp_new, (ssh_session), ());
    MOCK_METHOD(int, sftp_init, (sftp_session), ());
    MOCK_METHOD(void, sftp_free, (sftp_session), ());
    MOCK_METHOD(sftp_attributes, sftp_stat, (sftp_session, const char*), ());
    MOCK_METHOD(int, sftp_mkdir, (sftp_session, const char*, mode_t), ());
    MOCK_METHOD(int, sftp_unlink, (sftp_session, const char*), ());
    MOCK_METHOD(int, sftp_rmdir, (sftp_session, const char*), ());
    MOCK_METHOD(sftp_dir, sftp_opendir, (sftp_session, const char*), ());
    MOCK_METHOD(sftp_attributes, sftp_readdir, (sftp_session, sftp_dir), ());
    MOCK_METHOD(int, sftp_closedir, (sftp_dir), ());
    MOCK_METHOD(int, sftp_rename, (sftp_session, const char*, const char*), ());
    MOCK_METHOD(sftp_file, sftp_open, (sftp_session, const char*, int, mode_t), ());
    MOCK_METHOD(int, sftp_close, (sftp_file), ());
    MOCK_METHOD(int, sftp_read, (sftp_file, void*, size_t), ());
    MOCK_METHOD(int, sftp_write, (sftp_file, const void*, size_t), ());
};

extern MockSSHSession* g_mockSSHSession;

ssh_session ssh_new() {
    return g_mockSSHSession->ssh_new();
}

int ssh_options_set(ssh_session session, int type, const void* value) {
    return g_mockSSHSession->ssh_options_set(session, type, value);
}

int ssh_connect(ssh_session session) {
    return g_mockSSHSession->ssh_connect(session);
}

void ssh_disconnect(ssh_session session) {
    g_mockSSHSession->ssh_disconnect(session);
}

void ssh_free(ssh_session session) {
    g_mockSSHSession->ssh_free(session);
}

int ssh_userauth_password(ssh_session session, const char* username, const char* password) {
    return g_mockSSHSession->ssh_userauth_password(session, username, password);
}

ssh_channel ssh_channel_new(ssh_session session) {
    return g_mockSSHSession->ssh_channel_new(session);
}

int ssh_channel_open_session(ssh_channel channel) {
    return g_mockSSHSession->ssh_channel_open_session(channel);
}

int ssh_channel_request_exec(ssh_channel channel, const char* cmd) {
    return g_mockSSHSession->ssh_channel_request_exec(channel, cmd);
}

int ssh_channel_read(ssh_channel channel, void* buffer, uint32_t count, int is_stderr) {
    return g_mockSSHSession->ssh_channel_read(channel, buffer, count, is_stderr);
}

void ssh_channel_send_eof(ssh_channel channel) {
    g_mockSSHSession->ssh_channel_send_eof(channel);
}

void ssh_channel_close(ssh_channel channel) {
    g_mockSSHSession->ssh_channel_close(channel);
}

void ssh_channel_free(ssh_channel channel) {
    g_mockSSHSession->ssh_channel_free(channel);
}

const char* ssh_get_error(ssh_session session) {
    return g_mockSSHSession->ssh_get_error(session);
}

sftp_session sftp_new(ssh_session session) {
    return g_mockSSHSession->sftp_new(session);
}

int sftp_init(sftp_session sftp) {
    return g_mockSSHSession->sftp_init(sftp);
}

void sftp_free(sftp_session sftp) {
    g_mockSSHSession->sftp_free(sftp);
}

sftp_attributes sftp_stat(sftp_session sftp, const char* path) {
    return g_mockSSHSession->sftp_stat(sftp, path);
}

int sftp_mkdir(sftp_session sftp, const char* path, mode_t mode) {
    return g_mockSSHSession->sftp_mkdir(sftp, path, mode);
}

int sftp_unlink(sftp_session sftp, const char* path) {
    return g_mockSSHSession->sftp_unlink(sftp, path);
}

int sftp_rmdir(sftp_session sftp, const char* path) {
    return g_mockSSHSession->sftp_rmdir(sftp, path);
}

sftp_dir sftp_opendir(sftp_session sftp, const char* path) {
    return g_mockSSHSession->sftp_opendir(sftp, path);
}

sftp_attributes sftp_readdir(sftp_session sftp, sftp_dir dir) {
    return g_mockSSHSession->sftp_readdir(sftp, dir);
}

int sftp_closedir(sftp_dir dir) {
    return g_mockSSHSession->sftp_closedir(dir);
}

int sftp_rename(sftp_session sftp, const char* oldpath, const char* newpath) {
    return g_mockSSHSession->sftp_rename(sftp, oldpath, newpath);
}

sftp_file sftp_open(sftp_session sftp, const char* path, int access_type, mode_t mode) {
    return g_mockSSHSession->sftp_open(sftp, path, access_type, mode);
}

int sftp_close(sftp_file file) {
    return g_mockSSHSession->sftp_close(file);
}

int sftp_read(sftp_file file, void* buffer, size_t count) {
    return g_mockSSHSession->sftp_read(file, buffer, count);
}

int sftp_write(sftp_file file, const void* buffer, size_t count) {
    return g_mockSSHSession->sftp_write(file, buffer, count);
}

#endif // SSHCLIENT_MOCK_HPP
