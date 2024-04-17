#include "Server.hpp"
#include "Log.hpp"


void Server::unregisterClientToChannel(std::string unregisterChannnel, int fd)
{
	users[fd].eraseChannelList(channels[unregisterChannnel]);
	channels[unregisterChannnel].removeUser(users[fd]);
}


void Server::executePart(Message message, int fd)
{
	if (message.getParameters().empty() || !message.getParameters()[0].size())
		sendServerRpl(fd, ERR_NEEDMOREPARAMS(users[fd].getUserNickName(), "PART"));
	User client = users[fd];
	std::vector<std::string> argChannels = mySplit(message.getParameters()[0], ',');
	TrimVectorWhiteSpace(argChannels, "\n\r\t\b ");
	for (size_t i = 0; i < argChannels.size(); i++)
	{
		if (!isChannel(argChannels[i].substr(1))) 
			sendServerRpl(fd, ERR_NOSUCHCHANNEL(users[fd].getUserNickName(), argChannels[i].substr(1)));
		else 
		{
			if (!client.isChannel(argChannels[i].substr(1)))
				sendServerRpl(fd, ERR_NOTONCHANNEL(users[fd].getUserNickName(), argChannels[i].substr(1)));
			else
			{
				sendServerRpl(fd, RPL_PART(user_id(users[fd].getUserNickName(), users[fd].getUserName()), argChannels[i].substr(1), message.getTrailing()));
				std::vector<User*> channel_users = channels[argChannels[i].substr(1)].getChannelMembers();
                for (std::vector<User*>::iterator it = channel_users.begin(); it != channel_users.end(); it++)
                {
                    if ((*it)->getUserSockId() != fd)
                        sendServerRpl((*it)->getUserSockId(), RPL_PART(user_id(users[fd].getUserNickName(), users[fd].getUserName()), argChannels[i].substr(1), message.getTrailing()));
                }
				unregisterClientToChannel(argChannels[i].substr(1), fd);
			}
		}
	}
}