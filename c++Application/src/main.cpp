#include "../inc/ExchangeClient.h"
#include "../inc/PacketParser.h"
#include "../inc/PacketUtils.h"

int main() {
    try {
        ExchangeClient client("127.0.0.1", 3000);
        if (!client.connectToServer()) {
            return 1;
        }
        client.sendInitialRequest();
        client.receiveInitialData();
        client.requestMissingPackets();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}