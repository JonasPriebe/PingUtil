#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

int pingloop =1;

int checksum(void *buffer, int length) {
    u_int16_t *word = static_cast<u_int16_t *>(buffer);
    unsigned int sum = 0;
    for (sum = 0 ; length > 1 ; length -= 2) {

        sum += *word++;
    }
    if (length ==1) {
        sum += *(unsigned char*)word;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return  ~sum & 0xFFFF;
}

void interruptHandler (int sig) {
    pingloop = 0;
}

struct icmpheader {
        u_int8_t type; // 1 byt etype of ICMP message
        u_int8_t code; // 1 byte Code
        u_int16_t checksum; // 2 bytes Checksum
        u_int16_t id; // Identifier for matching
        u_int16_t seq; // Identifier for sequence tracking
    };

void sendEchoPing(int icmpSocket, struct sockaddr_in *ping_addr) {
    icmpheader header = {8,0,0,1234,10};
    header.checksum = checksum(&header,sizeof(header));
    socklen_t addressLength;
    if (sendto(icmpSocket, &header, sizeof(header), 0, reinterpret_cast<struct sockaddr *>(ping_addr), sizeof(*ping_addr)) < 0) {
        perror("sendto failed");
        exit(1);
    }
    std::cout << "ICMP request sent successfully!" << std::endl;
    char recvBuffer[128];
    ssize_t response = recvfrom(icmpSocket, recvBuffer, sizeof(recvBuffer), 0, reinterpret_cast<struct sockaddr *>(ping_addr), &addressLength);
    if (response < 0) {
        perror("recvfrom failed");
        exit(1);
    }
    std::cout << "Received " << response << " bytes" << '\n';
    for (int i =0; i < response ; i++) {
        std::cout << std::hex << (int) (unsigned char) recvBuffer[i] << " ";
    }
}




int main(int argc, char *argv[]){
    char *ip_address;
    struct timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    const int rawSocket = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    if (rawSocket < 0) {
        perror("Socket creation failed!");
        return 1;
    }
    if (setsockopt(rawSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }

    std::cout << "Socket has been created." << '\n';
    struct sockaddr_in destinationAddress {};
    destinationAddress.sin_family = AF_INET;
    destinationAddress.sin_port = 0;
    destinationAddress.sin_addr.s_addr = inet_addr("1.1.1.1");
    sendEchoPing(rawSocket,&destinationAddress);
    return 0;
}