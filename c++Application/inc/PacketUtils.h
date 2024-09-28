#ifndef PACKET_UTILS_H
#define PACKET_UTILS_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <arpa/inet.h> 

class PacketUtils {
public:
    static int32_t ntohl32(uint32_t bigEndianValue);
    static std::vector<uint8_t> createPayload(uint8_t callType, uint8_t sequenceNumber = 0);
};

const uint8_t CALL_TYPE_STREAM_ALL_PACKETS = 1;
const uint8_t CALL_TYPE_RESEND_PACKET = 2;

#endif 
