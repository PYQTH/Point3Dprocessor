#include <string>
#include <exception>

class IllegalStateException : public std::exception
{
    private:
        std::string msg;

    public:
        IllegalStateException(const std::string& message = "") : msg(message)
        {
        }

        const char * what() const noexcept
        {
            return msg.c_str();
        }
};
