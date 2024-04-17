/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nibenoit <nibenoit@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/21 16:58:53 by nwyseur           #+#    #+#             */
/*   Updated: 2023/11/30 13:53:25 by nibenoit         ###   ########.fr       */
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


void Server::sendInvitation(Message message, int fd)
{
	std::string	client;
	std::string	channel;
	bool		channelExist;

	client = message.getParameters()[0];
	channel = message.getParameters()[1];

	if (client.empty() || channel.empty() || channel.find('#') == channel.npos)
	{
		sendServerRpl(fd, ERR_NEEDMOREPARAMS(users[fd].getUserNickName(), message.getCommande()));
		return;
	}
	channel.erase(channel.find("#"), 1);
	// Check if the channel exists
	std::map<std::string, Channel>::iterator it;
	it = channels.find(channel);
	if (it == channels.end())
		channelExist = false;
	else
		channelExist = true;
	
	// Check if the invuted user exists
	if (!userExistName(client))
	{
		sendServerRpl(fd, ERR_NOSUCHNICK(users[fd].getUserNickName(), client));
		return;
	}

	if (channelExist == true)
	{
		// Check that the person inviting is a member of said channel
		std::vector<User*> theOneWhoInvite = channels[channel].getChannelMembers();
		size_t i = 0;
		while (i < theOneWhoInvite.size())
		{
			if (*(theOneWhoInvite)[i] == users[fd])
				break;
			i++;
		}
		if (i == theOneWhoInvite.size())
		{
			sendServerRpl(fd, ERR_NOTONCHANNEL(users[fd].getUserNickName(), channel));
			return;
		}

		// Check that the person inviting is an operator
		std::vector<User*> operatorsList = channels[channel].getChannelOperators();
		size_t k = 0;
		while (k < operatorsList.size())
		{
			if (*(operatorsList)[i] == users[fd])
				break;
			k++;
		}
		if (k == theOneWhoInvite.size())
		{
			sendServerRpl(fd, ERR_CHANOPRIVSNEEDED(users[fd].getUserNickName(), channel));
			return;
		}

		// Check that the invited user is not already on the channel
		std::vector<User*> userInvited = channels[channel].getChannelMembers();
		size_t j = 0;
		while (j < userInvited.size())
		{
			if ((userInvited)[j]->getUserNickName() == client)
			{
				sendServerRpl(fd, ERR_USERONCHANNEL(users[fd].getUserNickName(), client, channel));
				return;
			}
			j++;
		}
	}

	// Check that the person invited is already on the invited list
	std::vector<User*> InvitedList = channels[channel].getChannelInvitedUsers();
	size_t i = 0;
	while (i < InvitedList.size())
	{
		if (*(InvitedList)[i] == users[getUserIdByNickName(client)])
		{
			sendServerRpl(fd, ERR_USERONCHANNEL(users[fd].getUserNickName(), client, channel));
			return;
		}
		i++;
	}
	if (i == InvitedList.size())
	{
		channels[channel].addInvitedUser(users[getUserIdByNickName(client)]);
	}

	// If all checks are successful => send a RPL_INVITING + invite to the inviting user 
	sendServerRpl(fd, RPL_INVITING(user_id(users[fd].getUserNickName(), users[fd].getUserName()), users[fd].getUserNickName(), client, channel));
	std::map<int, User>::iterator itu;
	for (itu = users.begin(); itu != users.end(); itu++)
	{
		if (itu->second.getUserNickName() == client)
		{
			sendServerRpl(itu->second.getUserSockId(), RPL_INVITE(user_id(users[fd].getUserNickName(), users[fd].getUserName()), itu->second.getUserNickName(), channel));
			return;
		}
	}
}


bool Server::userExistName(std::string user)
{
	std::map<int, User>::iterator it;
	for (it = users.begin(); it != users.end(); it++)
	{
		if (it->second.getUserNickName() == user)
			return (true);
	}
	return (false);
}