# include "Server.hpp"

std::string incrementString(const std::string& baseString, int number) 
{
    std::ostringstream oss;
    oss << number;
    std::string numberString = oss.str();

    std::string result = baseString + numberString;

    return result;
}

std::string Server::changeNickname(std::string nickNameTochange, std::string NameTochange, int fd)
{
    int i = 1;
    std::string tmp(nickNameTochange);

    while (userExistName(tmp))
    {
        tmp = incrementString(nickNameTochange, i);
        i++;
    }
    sendServerRpl(fd, RPL_NICK(nickNameTochange, NameTochange, tmp));
    return (tmp);
}
