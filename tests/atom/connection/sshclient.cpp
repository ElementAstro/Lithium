#if __has_include(<libssh/libssh.h>)

#include "atom/connection/sshclient.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "sshclient_mock.hpp"

using namespace atom::connection;
using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

MockSSHSession* g_mockSSHSession = nullptr;

class SSHClientTest : public ::testing::Test {
protected:
    void SetUp() override { g_mockSSHSession = new MockSSHSession(); }

    void TearDown() override {
        delete g_mockSSHSession;
        g_mockSSHSession = nullptr;
    }
};

TEST_F(SSHClientTest, Connect_Success) {
    SSHClient client("localhost", 22);

    ssh_session mockSession = (ssh_session)1;
    sftp_session mockSftpSession = (sftp_session)2;

    EXPECT_CALL(*g_mockSSHSession, ssh_new()).WillOnce(Return(mockSession));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_HOST, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_PORT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_USER, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_TIMEOUT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_connect(mockSession))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_userauth_password(mockSession, _, _))
        .WillOnce(Return(SSH_AUTH_SUCCESS));
    EXPECT_CALL(*g_mockSSHSession, sftp_new(mockSession))
        .WillOnce(Return(mockSftpSession));
    EXPECT_CALL(*g_mockSSHSession, sftp_init(mockSftpSession))
        .WillOnce(Return(SSH_OK));

    client.Connect("username", "password");

    EXPECT_TRUE(client.IsConnected());
}

TEST_F(SSHClientTest, Connect_Failure) {
    SSHClient client("localhost", 22);

    ssh_session mockSession = (ssh_session)1;

    EXPECT_CALL(*g_mockSSHSession, ssh_new()).WillOnce(Return(mockSession));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_HOST, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_PORT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_USER, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_TIMEOUT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_connect(mockSession))
        .WillOnce(Return(SSH_ERROR));
    EXPECT_CALL(*g_mockSSHSession, ssh_get_error(mockSession))
        .WillOnce(Return("Failed to connect"));

    EXPECT_THROW(client.Connect("username", "password"), std::runtime_error);

    EXPECT_FALSE(client.IsConnected());
}

TEST_F(SSHClientTest, Disconnect) {
    SSHClient client("localhost", 22);

    ssh_session mockSession = (ssh_session)1;
    sftp_session mockSftpSession = (sftp_session)2;

    EXPECT_CALL(*g_mockSSHSession, ssh_new()).WillOnce(Return(mockSession));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_HOST, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_PORT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_USER, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_TIMEOUT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_connect(mockSession))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_userauth_password(mockSession, _, _))
        .WillOnce(Return(SSH_AUTH_SUCCESS));
    EXPECT_CALL(*g_mockSSHSession, sftp_new(mockSession))
        .WillOnce(Return(mockSftpSession));
    EXPECT_CALL(*g_mockSSHSession, sftp_init(mockSftpSession))
        .WillOnce(Return(SSH_OK));

    client.Connect("username", "password");
    EXPECT_TRUE(client.IsConnected());

    EXPECT_CALL(*g_mockSSHSession, sftp_free(mockSftpSession));
    EXPECT_CALL(*g_mockSSHSession, ssh_disconnect(mockSession));
    EXPECT_CALL(*g_mockSSHSession, ssh_free(mockSession));

    client.Disconnect();
    EXPECT_FALSE(client.IsConnected());
}

TEST_F(SSHClientTest, ExecuteCommand_Success) {
    SSHClient client("localhost", 22);

    ssh_session mockSession = (ssh_session)1;
    ssh_channel mockChannel = (ssh_channel)2;

    EXPECT_CALL(*g_mockSSHSession, ssh_new()).WillOnce(Return(mockSession));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_HOST, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_PORT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_USER, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_TIMEOUT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_connect(mockSession))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_userauth_password(mockSession, _, _))
        .WillOnce(Return(SSH_AUTH_SUCCESS));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_new(mockSession))
        .WillOnce(Return(mockChannel));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_open_session(mockChannel))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_request_exec(mockChannel, "ls"))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_read(mockChannel, _, _, _))
        .WillOnce(DoAll(SetArgPointee<1>('A'), Return(1)))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_send_eof(mockChannel));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_close(mockChannel));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_free(mockChannel));

    client.Connect("username", "password");
    auto future = client.executeCommandAsync("ls");
    auto result = future.get();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "A");
}

TEST_F(SSHClientTest, ExecuteCommand_Failure) {
    SSHClient client("localhost", 22);

    ssh_session mockSession = (ssh_session)1;
    ssh_channel mockChannel = (ssh_channel)2;

    EXPECT_CALL(*g_mockSSHSession, ssh_new()).WillOnce(Return(mockSession));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_HOST, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_PORT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_USER, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession,
                ssh_options_set(mockSession, SSH_OPTIONS_TIMEOUT, _))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_connect(mockSession))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_userauth_password(mockSession, _, _))
        .WillOnce(Return(SSH_AUTH_SUCCESS));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_new(mockSession))
        .WillOnce(Return(mockChannel));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_open_session(mockChannel))
        .WillOnce(Return(SSH_OK));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_request_exec(mockChannel, "ls"))
        .WillOnce(Return(SSH_ERROR));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_close(mockChannel));
    EXPECT_CALL(*g_mockSSHSession, ssh_channel_free(mockChannel));
    EXPECT_CALL(*g_mockSSHSession, ssh_get_error(mockSession))
        .WillOnce(Return("Failed to execute command"));

    client.Connect("username", "password");
    auto future = client.ExecuteCommandAsync("ls");
    EXPECT_THROW(future.get(), std::runtime_error);
}

#endif
