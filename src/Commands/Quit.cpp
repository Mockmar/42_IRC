/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nibenoit <nibenoit@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/29 17:26:50 by nwyseur           #+#    #+#             */
/*   Updated: 2023/12/01 15:27:14 by nibenoit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <poll.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Log.hpp"

void	Server::serverquit(Message message, int fd)
{
	std::string								  reason   = message.getTrailing();
	std::map<std::string, Channel>::iterator  chan	   = channels.begin();

	// inform the concerned user
	sendServerRpl(fd, RPL_QUIT(user_id(users[fd].getUserNickName(), users[fd].getUserName()), reason));
	// inform all the users that share a channel w/ the user quitting
	for (; chan != channels.end(); chan++) // check all channels
	{
		std::vector<User*>	chan_members = chan->second.getChannelMembers();
		std::vector<User*>::iterator	member		 = chan_members.begin();
		for (; member != chan_members.end(); member++) // check all chan_members
		{
			if ((*member) == &users[fd]) // erase user from the chan + inform the others 
			{
				chan->second.removeUser(users[fd]);
				broadcastQuitToChan(chan->second, users[fd], reason);
				break ;
			}
		}
	}
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        perror("epoll_ctl");
        exit(1);
    }
    users.erase(fd);
    close(fd);
	--_nb_clients;
    Log::info() << "Client disconnected" << '\n';
}

void	Server::broadcastQuitToChan(Channel& channel, User& user, std::string reason)
{
	std::vector<User*>  channelMembers = channel.getChannelMembers();
    std::vector<User*>::const_iterator member = channelMembers.begin();
    while (member != channelMembers.end())
    {
        sendServerRpl((*member)->getUserSockId(), RPL_QUIT(user_id(user.getUserNickName(), user.getUserName()), reason));
        member++;
    }	
}
