#ifndef TCP_PROXY_HPP
#define TCP_PROXY_HPP

#include <string>

/**
 * @brief Forwards data from the source socket to the destination socket.
 *
 * This function reads data from the source socket and writes it to the
 * destination socket. It continues to forward data until the source socket is
 * closed or an error occurs.
 *
 * @param srcSockfd The file descriptor of the source socket.
 * @param dstSockfd The file descriptor of the destination socket.
 */
void forwardData(int srcSockfd, int dstSockfd);

/**
 * @brief Starts the TCP proxy server.
 *
 * This function starts a TCP proxy server that listens on the specified source
 * IP and port, and forwards traffic to the specified destination IP and port.
 *
 * @param srcIp The source IP address to listen on.
 * @param srcPort The source port to listen on.
 * @param dstIp The destination IP address to forward traffic to.
 * @param dstPort The destination port to forward traffic to.
 */
void startProxyServer(const std::string &srcIp, int srcPort,
                      const std::string &dstIp, int dstPort);

/**
 * @brief Signal handler for gracefully shutting down the proxy server.
 *
 * This function handles signals such as SIGINT and SIGTERM to gracefully shut
 * down the proxy server.
 *
 * @param signal The signal number received.
 */
void signalHandler(int signal);

#endif  // TCP_PROXY_HPP