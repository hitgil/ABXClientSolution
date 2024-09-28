#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <set>
#include <algorithm>

const uint8_t CALL_TYPE_STREAM_ALL_PACKETS = 1;
const uint8_t CALL_TYPE_RESEND_PACKET = 2;

class PacketUtils {
public:
    static int32_t ntohl32(uint32_t bigEndianValue) {
        return ntohl(bigEndianValue);
    }

    static std::vector<uint8_t> createPayload(uint8_t callType, uint8_t sequenceNumber = 0) {
        std::vector<uint8_t> payLoad(2);
        payLoad[0] = callType;
        payLoad[1] = (callType == CALL_TYPE_RESEND_PACKET) ? sequenceNumber : 0;
        return payLoad;
    }
};

class PacketParser {
public:
    static void parseSinglePacket(const std::vector<uint8_t>& packet) {
        if (packet.size() != 17) {
            std::cerr << "Invalid packet size." << std::endl;
            return;
        }

        // Parse Symbol (4 bytes)
        char symbol[5];
        std::memcpy(symbol, &packet[0], 4);
        symbol[4] = '\0';  // Null-terminate the string

        // Parse Buy/Sell Indicator (1 byte)
        char buySellIndicator = packet[4];

        // Parse Quantity (4 bytes, Big Endian to native)
        uint32_t quantityBigEndian;
        std::memcpy(&quantityBigEndian, &packet[5], 4);
        int32_t quantity = PacketUtils::ntohl32(quantityBigEndian);

        // Parse Price (4 bytes, Big Endian to native)
        uint32_t priceBigEndian;
        std::memcpy(&priceBigEndian, &packet[9], 4);
        int32_t price = PacketUtils::ntohl32(priceBigEndian);

        // Parse Packet Sequence (4 bytes, Big Endian to native)
        uint32_t packetSeqBigEndian;
        std::memcpy(&packetSeqBigEndian, &packet[13], 4);
        int32_t packetSeq = PacketUtils::ntohl32(packetSeqBigEndian);

        std::cout << "Symbol: " << symbol << "\n";
        std::cout << "Buy/Sell: " << buySellIndicator << "\n";
        std::cout << "Quantity: " << quantity << "\n";
        std::cout << "Price: " << price << "\n";
        std::cout << "Packet Sequence: " << packetSeq << "\n\n";
    }

    static void parseResponse(const std::vector<uint8_t>& response, std::set<int>& receivedSeq, std::set<int>& missingSeq) {
        size_t packetSize = 17;
        size_t numPackets = response.size() / packetSize;

        if (response.size() % packetSize != 0) {
            std::cerr << "Warning: Response size is not a multiple of expected packet size.\n";
            return;
        }

        for (size_t i = 0; i < numPackets; ++i) {
            // Extract the current packet from the response
            std::vector<uint8_t> packet(response.begin() + i * packetSize, response.begin() + (i + 1) * packetSize);

            // Parse the single packet
            parseSinglePacket(packet);

            // Extract the packet sequence number from the parsed packet
            uint32_t packetSeqBigEndian;
            std::memcpy(&packetSeqBigEndian, &packet[13], 4);
            int32_t packetSeq = PacketUtils::ntohl32(packetSeqBigEndian);

            // Track received packet sequence
            receivedSeq.insert(packetSeq);
        }

        // Find missing sequences
        int lastSeq = *receivedSeq.rbegin();
        for (int seq = 1; seq < lastSeq; ++seq) {
            if (receivedSeq.find(seq) == receivedSeq.end()) {
                missingSeq.insert(seq);
            }
        }
    }
};

class ExchangeClient {
private:
    int sock;
    struct sockaddr_in server;
    std::set<int> receivedSeq;
    std::set<int> missingSeq;
    std::vector<uint8_t> response;

public:
    ExchangeClient(const std::string& serverIP, int port) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            throw std::runtime_error("Could not create socket");
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = inet_addr(serverIP.c_str());
    }

    ~ExchangeClient() {
        close(sock);
    }

    bool connectToServer() {
        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
            std::cerr << "Connection failed" << std::endl;
            return false;
        }
        std::cout << "Connected to server" << std::endl;
        return true;
    }

    void sendInitialRequest() {
        std::vector<uint8_t> payload = PacketUtils::createPayload(CALL_TYPE_STREAM_ALL_PACKETS);
        if (send(sock, payload.data(), payload.size(), 0) < 0) {
            throw std::runtime_error("Send failed");
        }
        std::cout << "Data sent to server.\n";
    }

    void receiveInitialData() {
        response.resize(1024);
        int totalBytesReceived = 0;

        while (true) {
            int bytesReceived = recv(sock, response.data() + totalBytesReceived, response.size() - totalBytesReceived, 0);
            if (bytesReceived < 0) {
                throw std::runtime_error("Receive failed");
            }
            if (bytesReceived == 0) {
                break;  // Server closed the connection
            }
            totalBytesReceived += bytesReceived;

            if (totalBytesReceived >= response.size()) {
                response.resize(response.size() * 2);
            }
        }

        std::cout << "Data received from server: " << totalBytesReceived << " bytes\n";
        response.resize(totalBytesReceived);

        // Parse initial response
        PacketParser::parseResponse(response, receivedSeq, missingSeq);
    }

    void requestMissingPackets() {
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

            // Request missing packet and get the response
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

            // Append the missing packet response to the original response
            response.insert(response.end(), missingPacketResponse.begin(), missingPacketResponse.end());

            close(resendSock);  // Close connection after each request
        }

        // Parse entire response after appending missing packets
        PacketParser::parseResponse(response, receivedSeq, missingSeq);
    }

    void closeConnection() {
        close(sock);
    }
};

int main() {
    try {
        ExchangeClient client("127.0.0.1", 3000);

        if (!client.connectToServer()) {
            return 1;
        }

        client.sendInitialRequest();
        client.receiveInitialData();
        client.requestMissingPackets();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
