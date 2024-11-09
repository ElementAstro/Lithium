#ifndef TCP_PROXY_HPP
#define TCP_PROXY_HPP

#include <string>

void forwardData(int srcSockfd, int dstSockfd);
void startProxyServer(const std::string &srcIp, int srcPort, const std::string &dstIp, int dstPort);
void signalHandler(int signal);

#endif  // TCP_PROXY_HPP
