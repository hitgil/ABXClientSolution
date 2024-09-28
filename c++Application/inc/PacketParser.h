#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include <iostream>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>  
#include "PacketUtils.h"  
using json = nlohmann::json;

class PacketParser {
public:
    struct PacketData {
        std::string symbol;
        char buySellIndicator;
        int32_t quantity;
        int32_t price;
        int32_t packetSeq;
    };

    static PacketData parseSinglePacket(const std::vector<uint8_t>& packet);
    static void parseResponse(const std::vector<uint8_t>& response, std::set<int>& receivedSeq, 
                              std::set<int>& missingSeq, std::vector<PacketData>& packets);
    static json createJson(const std::vector<PacketData>& packets);
};

#endif 
