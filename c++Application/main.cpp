#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

const uint8_t CALL_TYPE_STREAM_ALL_PACKETS = 1;
const uint8_t CALL_TYPE_RESEND_PACKET = 2;


// Function to convert from Big Endian to native endianness
int32_t ntohl32(uint32_t bigEndianValue) {
    return ntohl(bigEndianValue);
}

void parseResponse(const std::vector<uint8_t>& response) {
    size_t packetSize = 17;  // Each packet is 17 bytes
    size_t numPackets = response.size() / packetSize;

    for (size_t i = 0; i < numPackets; ++i) {
        size_t offset = i * packetSize;

        // Parse Symbol (4 bytes)
        char symbol[5];
        std::memcpy(symbol, &response[offset], 4);
        symbol[4] = '\0';  // Null-terminate the string

        // Parse Buy/Sell Indicator (1 byte)
        char buySellIndicator = response[offset + 4];

        // Parse Quantity (4 bytes, Big Endian to native)
        uint32_t quantityBigEndian;
        std::memcpy(&quantityBigEndian, &response[offset + 5], 4);
        int32_t quantity = ntohl32(quantityBigEndian);

        // Parse Price (4 bytes, Big Endian to native)
        uint32_t priceBigEndian;
        std::memcpy(&priceBigEndian, &response[offset + 9], 4);
        int32_t price = ntohl32(priceBigEndian);

        // Parse Packet Sequence (4 bytes, Big Endian to native)
        uint32_t packetSeqBigEndian;
        std::memcpy(&packetSeqBigEndian, &response[offset + 13], 4);
        int32_t packetSeq = ntohl32(packetSeqBigEndian);

        // Print parsed data
        std::cout << "Packet " << i + 1 << ":\n";
        std::cout << "Symbol: " << symbol << "\n";
        std::cout << "Buy/Sell: " << buySellIndicator << "\n";
        std::cout << "Quantity: " << quantity << "\n";
        std::cout << "Price: " << price << "\n";
        std::cout << "Packet Sequence: " << packetSeq << "\n\n";
    }
}



std::vector<uint8_t> createPayload(uint8_t callType, uint8_t sequenceNumber = 0)
{
    std::vector<uint8_t> payLoad(2);
    payLoad[0] = callType;
    payLoad[1] = (callType == 2) ? sequenceNumber : 0;
    return payLoad;
}

int main()
{

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        std::cerr << "Could not create socket" << std::endl;
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(3000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    uint8_t callType = CALL_TYPE_STREAM_ALL_PACKETS;
    uint8_t sequenceNumber = 0;

    std::vector<uint8_t> payload = createPayload(callType, sequenceNumber);
    if (send(sock, payload.data(), payload.size(), 0) < 0)
    {
        std::cerr << "send failed" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Data sent to server: " << std::endl;


     // Buffer to store response
    std::vector<uint8_t> response(1024);  // Adjust size as needed
    int bytesReceived = recv(sock, response.data(), response.size(), 0);
    if (bytesReceived < 0) {
        std::cerr << "Receive failed" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Data received from server: " << bytesReceived << " bytes\n";

    // Parse the response
    response.resize(bytesReceived);  // Resize the buffer to actual bytes received
    parseResponse(response);

    return 0;
}