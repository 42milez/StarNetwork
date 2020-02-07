#ifndef P2P_TECHDEMO_CORE_INITIALIZATION_EXCEPTION_H
#define P2P_TECHDEMO_CORE_INITIALIZATION_EXCEPTION_H

class InitializationException
{
public:
    explicit InitializationException(std::string &&msg);

    const char *what() const noexcept;

private:
    std::string _msg;
};

#endif // P2P_TECHDEMO_CORE_INITIALIZATION_EXCEPTION_H
