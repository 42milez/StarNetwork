#include "string.h"

bool is_valid_hex_number(const std::string &str, bool with_prefix)
{
    int len = str.size();

    if (len == 0)
    {
        return false;
    }

    int from = 0;

    if (len != 1 && (str[0] == '+' || str[0] == '-'))
    {
        from++;
    }

    if (with_prefix)
    {
        if (len < 3)
        {
            return false;
        }

        if (str[from] != '0' || str[from + 1] != 'x')
        {
            return false;
        }

        from += 2;
    }

    for (auto i = from; i < len; i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return false;
        }
    }

    return true;
}

bool is_valid_integer(const std::string &str)
{
    int len = str.size();

    if (len == 0)
    {
        return false;
    }

    int from = 0;

    if (len != 1 && (str[0] == '+' || str[0] == '-'))
    {
        from++;
    }

    for (auto i = from; i < len; i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return false;
        }
    }

    return true;
}
