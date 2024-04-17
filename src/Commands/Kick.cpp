#include "Server.hpp"

void Server::executeKick(Message msg, int fd)
{
	if (msg.getParameters().empty() || msg.getParameters().size() < 2 || !msg.getParameters()[0].size() || !msg.getParameters()[1].size())
		return(sendServerRpl(fd, ERR_NEEDMOREPARAMS(users[fd].getUserNickName(), "KICK")));
	
	std::string ChanToKick = msg.getParameters()[0];
	std::string kickUser = msg.getParameters()[1];

	if (ChanToKick.find(",") != std::string::npos || kickUser.find(",") != std::string::npos)
		return (sendServerRpl(fd, ERR_BADMASK(users[fd].getUserNickName(), std::string("KICK ") + ChanToKick + std::string(" ") + kickUser)));
	
	if (!isChannel(ChanToKick.substr(1))) 
		return (sendServerRpl(fd, ERR_NOSUCHCHANNEL(users[fd].getUserNickName(), ChanToKick.substr(1))));

	int userfd = userNameToFd(kickUser);
	if (!userfd || !channels[ChanToKick.substr(1)].hasUser(users[userfd]))
		return (sendServerRpl(fd, ERR_USERNOTINCHANNEL(users[fd].getUserNickName(), kickUser, ChanToKick.substr(1))));
	
	if (!channels[ChanToKick.substr(1)].hasOp(users[fd]))
		return(sendServerRpl(fd, ERR_CHANOPRIVSNEEDED(users[fd].getUserNickName(), ChanToKick.substr(1))));

	if (userfd == fd)
		return (sendServerRpl(fd, ERR_CANOTAUTOKICK(users[fd].getUserName(), ChanToKick.substr(1))));

	sendServerRpl(userfd, RPL_PART(user_id(users[userfd].getUserNickName(), users[userfd].getUserName()), ChanToKick.substr(1), msg.getTrailing()));
	sendServerRpl(userfd, RPL_KICK(user_id(users[userfd].getUserNickName(), users[userfd].getUserName()), ChanToKick.substr(1), users[userfd].getUserNickName(), msg.getTrailing()));
	unregisterClientToChannel(ChanToKick.substr(1), userfd);
	channels[ChanToKick.substr(1)].addKickedUser(users[userfd]);
	std::vector<User*> channel_users = channels[ChanToKick.substr(1)].getChannelMembers();
    for (std::vector<User*>::iterator it = channel_users.begin(); it != channel_users.end(); it++)
    {
        if ((*it)->getUserSockId() != userfd)
            sendServerRpl((*it)->getUserSockId(), RPL_KICK(user_id(users[fd].getUserNickName(), users[fd].getUserName()), ChanToKick.substr(1), users[userfd].getUserNickName(), msg.getTrailing()));
    }
}