#ifndef P2P_TECHDEMO_LIB_CORE_INITIALIZATION_EXCEPTION_H_
#define P2P_TECHDEMO_LIB_CORE_INITIALIZATION_EXCEPTION_H_

class InitializationException
{
public:
    explicit InitializationException(std::string &&msg);

    const char *what() const noexcept;

private:
    std::string _msg;
};

#endif // P2P_TECHDEMO_LIB_CORE_INITIALIZATION_EXCEPTION_H_
