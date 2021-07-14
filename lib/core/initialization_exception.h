#ifndef STAR_NETWORK_LIB_CORE_INITIALIZATION_EXCEPTION_H_
#define STAR_NETWORK_LIB_CORE_INITIALIZATION_EXCEPTION_H_

namespace core
{
    class InitializationException
    {
      public:
        explicit InitializationException(std::string &&msg);

        const char *
        what() const noexcept;

      private:
        std::string _msg;
    };
} // namespace core

#endif // STAR_NETWORK_LIB_CORE_INITIALIZATION_EXCEPTION_H_
