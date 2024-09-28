#ifndef EXCHANGE_CLIENT_H
#define EXCHANGE_CLIENT_H

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <set>
#include <fstream>
#include <nlohmann/json.hpp>
#include "PacketParser.h" 
#include "PacketUtils.h"  
using json = nlohmann::json;

class ExchangeClient {
private:
    int sock;
    struct sockaddr_in server;
    std::set<int> receivedSeq;
    std::set<int> missingSeq;
    std::vector<uint8_t> response;
    std::vector<PacketParser::PacketData> packets;  

public:
    ExchangeClient(const std::string& serverIP, int port);
    ~ExchangeClient();
    
    bool connectToServer();
    void sendInitialRequest();
    void receiveInitialData();
    void requestMissingPackets();
    void createJsonFile();
};

#endif 