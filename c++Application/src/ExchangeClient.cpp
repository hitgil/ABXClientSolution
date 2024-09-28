#include "../inc/ExchangeClient.h"

ExchangeClient::ExchangeClient(const std::string& serverIP, int port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        throw std::runtime_error("Could not create socket");
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(serverIP.c_str());
}

ExchangeClient::~ExchangeClient() {
    close(sock);
}

bool ExchangeClient::connectToServer() {
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return false;
    }
    std::cout << "Connected to server" << std::endl;
    return true;
}

void ExchangeClient::sendInitialRequest() {
    std::vector<uint8_t> payload = PacketUtils::createPayload(CALL_TYPE_STREAM_ALL_PACKETS);
    if (send(sock, payload.data(), payload.size(), 0) < 0) {
        throw std::runtime_error("Send failed");
    }
    std::cout << "Data sent to server.\n";
}

void ExchangeClient::receiveInitialData() {
    response.resize(1024);
    int totalBytesReceived = 0;

    while (true) {
        int bytesReceived = recv(sock, response.data() + totalBytesReceived, response.size() - totalBytesReceived, 0);
        if (bytesReceived < 0) {
            throw std::runtime_error("Receive failed");
        }
        if (bytesReceived == 0) {
            break; 
        }
        totalBytesReceived += bytesReceived;

        if (totalBytesReceived >= response.size()) {
            response.resize(response.size() * 2);
        }
    }

    std::cout << "Data received from server: " << totalBytesReceived << " bytes\n";
    response.resize(totalBytesReceived);

    
    PacketParser::parseResponse(response, receivedSeq, missingSeq, packets);
}

void ExchangeClient::requestMissingPackets() {
    for (int seq : missingSeq) {
        std::cout << "Requesting missing packet with sequence: " << seq << "\n";

        int resendSock = socket(AF_INET, SOCK_STREAM, 0);
        if (resendSock == -1) {
            std::cerr << "Could not create socket for resending packet" << std::endl;
            continue;
        }

        if (connect(resendSock, (struct sockaddr*)&server, sizeof(server)) < 0) {
            std::cerr << "Connection failed for resending packet" << std::endl;
            close(resendSock);
            continue;
        }

      
        std::vector<uint8_t> resendPayload = PacketUtils::createPayload(CALL_TYPE_RESEND_PACKET, static_cast<uint8_t>(seq));

        if (send(resendSock, resendPayload.data(), resendPayload.size(), 0) < 0) {
            std::cerr << "Failed to send missing packet request for sequence: " << seq << std::endl;
            close(resendSock);
            continue;
        }

        std::vector<uint8_t> missingPacketResponse(17);  // Expecting a single packet of 17 bytes
        int bytesReceived = recv(resendSock, missingPacketResponse.data(), missingPacketResponse.size(), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Failed to receive missing packet for sequence: " << seq << std::endl;
            close(resendSock);
            continue;
        }

        std::cout << "Received missing packet for sequence: " << seq << "\n";

        
        PacketParser::PacketData packetData = PacketParser::parseSinglePacket(missingPacketResponse);
        packets.push_back(packetData);

        close(resendSock);  
    }

    
    createJsonFile();
}

void ExchangeClient::createJsonFile() {
    json jsonArray = PacketParser::createJson(packets);
    std::ofstream outputFile("packets.json");
    outputFile << jsonArray.dump(4); 
    outputFile.close();
    std::cout << "JSON file created: packets.json\n";
}
