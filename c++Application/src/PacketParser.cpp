#include "../inc/PacketParser.h"
#include "../inc/PacketUtils.h"
PacketParser::PacketData PacketParser::parseSinglePacket(const std::vector<uint8_t> &packet)
{
    PacketData packetData;

    if (packet.size() != 17)
    {
        std::cerr << "Invalid packet size." << std::endl;
        return packetData;
    }

    char symbol[5];
    std::memcpy(symbol, &packet[0], 4);
    symbol[4] = '\0';
    packetData.symbol = symbol;

    packetData.buySellIndicator = packet[4];

    uint32_t quantityBigEndian;
    std::memcpy(&quantityBigEndian, &packet[5], 4);
    packetData.quantity = PacketUtils::ntohl32(quantityBigEndian);

    uint32_t priceBigEndian;
    std::memcpy(&priceBigEndian, &packet[9], 4);
    packetData.price = PacketUtils::ntohl32(priceBigEndian);

    uint32_t packetSeqBigEndian;
    std::memcpy(&packetSeqBigEndian, &packet[13], 4);
    packetData.packetSeq = PacketUtils::ntohl32(packetSeqBigEndian);

    return packetData;
}

void PacketParser::parseResponse(const std::vector<uint8_t> &response, std::set<int> &receivedSeq,
                                 std::set<int> &missingSeq, std::vector<PacketData> &packets)
{
    size_t packetSize = 17;
    size_t numPackets = response.size() / packetSize;

    if (response.size() % packetSize != 0)
    {
        std::cerr << "Warning: Response size is not a multiple of expected packet size.\n";
        return;
    }

    for (size_t i = 0; i < numPackets; ++i)
    {
        std::vector<uint8_t> packet(response.begin() + i * packetSize, response.begin() + (i + 1) * packetSize);

        PacketData packetData = parseSinglePacket(packet);
        packets.push_back(packetData);
        receivedSeq.insert(packetData.packetSeq);
    }

    int lastSeq = *receivedSeq.rbegin();
    for (int seq = 1; seq < lastSeq; ++seq)
    {
        if (receivedSeq.find(seq) == receivedSeq.end())
        {
            missingSeq.insert(seq);
        }
    }
}

json PacketParser::createJson(const std::vector<PacketData> &packets)
{
    json jsonArray = json::array();

    for (const auto &packet : packets)
    {
        json jsonObject;
        jsonObject["symbol"] = packet.symbol;
        jsonObject["buySellIndicator"] = packet.buySellIndicator;
        jsonObject["quantity"] = packet.quantity;
        jsonObject["price"] = packet.price;
        jsonObject["packetSeq"] = packet.packetSeq;
        jsonArray.push_back(jsonObject);
    }
    return jsonArray;
}
