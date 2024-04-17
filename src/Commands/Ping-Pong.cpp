#include "Server.hpp"

void Server::sendPong(Message msg, int fd)
{
    if (msg.getParameters().empty() || !msg.getParameters()[0].size())
		return (sendServerRpl(fd, ERR_NOORIGIN(user_id(users[fd].getUserNickName(), users[fd].getUserName()), users[fd].getUserNickName())));
	return (sendServerRpl(fd, RPL_PONG(user_id(users[fd].getUserNickName(), users[fd].getUserName()), msg.getParameters()[0])));
}
