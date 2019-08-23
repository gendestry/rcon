#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// RCON packet types
#define SERVERDATA_RESPONSE_VALUE 0
#define SERVERDATA_EXECCOMMAND 2
#define SERVERDATA_AUTH_RESPONSE 2
#define SERVERDATA_AUTH 3

#define PSIZE_OFFSET 0
#define PID_OFFSET 4
#define PTYPE_OFFSET 8
#define PBODY_OFFSET 12

struct Packet {
    unsigned char* data;
    unsigned int size;
};

int connectRCON(std::string host, int port, std::string pass);

Packet createRCONPacket(std::string cmd, int serverData);
void printPacketData(unsigned char* data, int size);
void printPacket(Packet p);
void sendAuth(int socket, std::string pass);
void sendCommand(int socket, std::string cmd);
void readResponse(int socket);

int main () {
    int socket = connectRCON("192.168.64.150", 25575, "rcon");
    // Connected
    for(int i = 0; i < 10; i++)
    	sendCommand(socket, "execute at bananal0rd run summon vex");
    //sendCommand(socket, "list");
    //sendCommand(socket, "version");

    close(socket);
    return 0;
}

int connectRCON(std::string host, int port, std::string pass) {
    int sockfd;
    sockaddr_in serv_addr;
    hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server = gethostbyname(host.c_str());

    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if(connect(sockfd, (sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        std::cout << "Connection failed" << std::endl;

    sendAuth(sockfd, pass);
    return sockfd;
}

Packet createRCONPacket(std::string cmd, int type) {
    unsigned int size = static_cast<unsigned char>(13 + cmd.length()) + 1;
    unsigned char* data = new unsigned char[size];
    memset(data, 0, size);
    
    data[PSIZE_OFFSET] = cmd.length() + 10;         // packet size
    data[PID_OFFSET] = 1;                           // id
    data[PTYPE_OFFSET] = type;                      // packet type
    for(int i = 0; i < cmd.length(); i++)
        data[PBODY_OFFSET + i] = (unsigned char)cmd.at(i);

    Packet packet;
    packet.data = data;
    packet.size = size;

    return packet;
}

void printPacketData(unsigned char* data, int size) {
    for(int i = 0; i < size; i++)
        std::cout << std::hex << (int)data[i] << " ";
    std::cout << std::dec << std::endl;
}

void printPacket(Packet p) {
    std::cout << "Hex: ";
    printPacketData(p.data, p.size);

    std::cout << "Size: " << p.size << std::endl << "Body: ";

    for(int i = PBODY_OFFSET; i < p.size - 1; i++)
        std::cout << p.data[i];
    std::cout << std::endl;
}

void sendAuth(int socket, std::string pass) {
    auto packet = createRCONPacket(pass, SERVERDATA_AUTH);
    //printPacket(packet);
    write(socket, packet.data, packet.size);
    delete packet.data;

    char buffer[14];
    memset(buffer, 0, 14);
    read(socket, buffer, 14);

    if(*(int*)(buffer + PID_OFFSET) < 0)
        std::cout << "Authentication error!"<< std::endl;
    else {
        std::cout << "Authentication successful!" << std::endl;
    }
}

void sendCommand(int socket, std::string cmd) {
    auto packet = createRCONPacket(cmd, SERVERDATA_EXECCOMMAND);
    //printPacket(packet);
    write(socket, packet.data, packet.size);
    delete packet.data;

    readResponse(socket);
}

void readResponse(int socket) {
    // Get packet size
    unsigned char bsize[4];
    memset(bsize, 0, 4);
    read(socket, bsize, 4);

    int size = *(int*)bsize;
    
    // Get response
    unsigned char buffer[size];
    memset(buffer, 0, size);
    read(socket, buffer, size);

    //printPacketData(buffer, size);

    for(int i = PBODY_OFFSET - 4; i < size - 1; i++)
        std::cout << buffer[i];
    std::cout << std::endl;
}
