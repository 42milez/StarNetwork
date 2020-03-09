#ifndef P2P_TECHDEMO_LIB_CORE_OS_H_
#define P2P_TECHDEMO_LIB_CORE_OS_H_

#include <string>
#include <cstdint>

class OS
{
public:
    OS();
    [[nodiscard]] uint64_t GetTicksUsec() const;
    inline uint32_t GetTicksMsec() { return GetTicksUsec() / 1000; };
    static inline uint64_t GetUnixTime() { return 0; };
    static inline std::string GetUserDataDir() { return "."; };

 private:
    uint64_t clock_start_;
    double clock_scale_;
};

#endif // P2P_TECHDEMO_LIB_CORE_OS_H_
