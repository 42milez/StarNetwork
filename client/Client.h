#ifndef P2P_TECHDEMO_PEER_H
#define P2P_TECHDEMO_PEER_H

#include <memory>
#include <string>

namespace peer {

  class Peer {
  public:
    bool init();

    void run();

    int request_token(const uint8_t *buf, uint32_t size);

    bool token_exists();

  private:
    std::string token_;
  };

} // namespace peer

#endif // P2P_TECHDEMO_PEER_H
