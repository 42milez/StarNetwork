#ifndef P2P_TECHDEMO_CORE_BASE_LOGGER_H
#define P2P_TECHDEMO_CORE_BASE_LOGGER_H

class Logger
{
private:
    std::shared_ptr <spdlog::logger> _logger;

public:
    template<class... Args>
    void info(Args... args)
    {
        _logger->info(args...);
    };

    template<class... Args>
    void warn(Args... args)
    {
        _logger->warn(args...);
    };

    template<class... Args>
    void error(Args... args)
    {
        _logger->error(args...);
    };

    template<class... Args>
    void critical(Args... args)
    {
        _logger->critical(args...);
    };

    Logger(const std::string &logger_name, const std::string &filename);
};

#endif // P2P_TECHDEMO_CORE_BASE_LOGGER_H
