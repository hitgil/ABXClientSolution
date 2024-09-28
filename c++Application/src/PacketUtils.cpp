#include "../inc/PacketUtils.h"

int32_t PacketUtils::ntohl32(uint32_t bigEndianValue) {
    return ntohl(bigEndianValue);
}

std::vector<uint8_t> PacketUtils::createPayload(uint8_t callType, uint8_t sequenceNumber) {
    std::vector<uint8_t> payLoad(2);
    payLoad[0] = callType;
    payLoad[1] = (callType == CALL_TYPE_RESEND_PACKET) ? sequenceNumber : 0;
    return payLoad;
}
