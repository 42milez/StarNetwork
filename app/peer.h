#ifndef P2P_TECHDEMO_PEER_H
#define P2P_TECHDEMO_PEER_H

#include <memory>
#include <string>

namespace peer
{
class Peer
{
public:
    bool Init();
    void Run();
    // int RequestToken(const uint8_t *buf, uint32_t size);
    // bool TokenExists();

private:
    std::string token_;
};
} // namespace peer

#endif // P2P_TECHDEMO_PEER_H
