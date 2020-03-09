#ifndef P2P_TECHDEMO_APP_PEER_H_
#define P2P_TECHDEMO_APP_PEER_H_

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

#endif // P2P_TECHDEMO_APP_PEER_H_
