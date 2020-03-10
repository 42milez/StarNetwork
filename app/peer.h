#ifndef P2P_TECHDEMO_APP_PEER_H_
#define P2P_TECHDEMO_APP_PEER_H_

#include <memory>
#include <string>

namespace app
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
} // namespace app

#endif // P2P_TECHDEMO_APP_PEER_H_
