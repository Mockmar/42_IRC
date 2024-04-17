/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nibenoit <nibenoit@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 15:12:23 by nwyseur           #+#    #+#             */
/*   Updated: 2023/11/30 15:37:36 by nibenoit         ###   ########.fr       */
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

void sendChanInfo(Channel& channel, User& user);
bool atLeastOneAlphaNum(std::string toTest);
void addClientToChannel(Channel& channel, User& client);
std::string getChannelName(std::string strToPars);
std::string	getChannelKeyword(std::string strToPars);

void Server::executeJoinOrder(Message message, int fd)
{
    std::string channel_list = message.getParameters()[0];
    std::string keyword_list;
    std::string channel;

    if (message.getParameters().size() >= 2)
        keyword_list = message.getParameters()[1];

    // Check if there is enough parameter
    if (atLeastOneAlphaNum(channel_list) == false)
    {
        sendServerRpl(fd, ERR_NEEDMOREPARAMS(users[fd].getUserNickName(), message.getCommande()));
        return;
    }
    while (atLeastOneAlphaNum(channel_list) == true)
    {
        channel.clear();
        channel = getChannelName(channel_list);
        size_t posr = channel.find('\r');
        if (posr != std::string::npos)
            channel = channel.erase(posr, std::string::npos);
        size_t posn = channel.find('\n');
        if (posn != std::string::npos)
            channel = channel.erase(posn, std::string::npos);

        // erase de la string le channel = "#foo,#bar" devient "#,#bar"
        channel_list.erase(channel_list.find(channel), channel.length());
        // Get Channel thanks to channel name
        std::map<std::string, Channel>::iterator it;
        it = channels.find(channel);

        int flag = 0;
        // Create channel if it doesn't exist
        if (it == channels.end())
        {
            addChannel(channel, users[fd]);
            users[fd].addChannelList(channels[channel]);
            flag = 1;
        }

        // Handle k mode
        else if (it->second.getChannelMode().find('k') != std::string::npos) // Si channel en mode +k
		{
			std::string key = getChannelKeyword(keyword_list);
			keyword_list.erase(keyword_list.find(key), key.length()); // on erase la key de la string
			if (key != it->second.getPassword())
			{
				sendServerRpl(fd, ERR_BADCHANNELKEY(users[fd].getUserNickName(), channel));
				continue; // on passe la suite, au prochain channel à ajouter síl y en a un
			}
		}

        // Handle i mode
        else if (it->second.getChannelMode().find('i') != std::string::npos) // Si channel en mode +i
		{
			std::vector<User*> InvitedList = it->second.getChannelInvitedUsers();
	        size_t i = 0;
	        while (i < InvitedList.size())
	        {
	        	if (*(InvitedList)[i] == users[fd])
	        	{
	        		break;
	        	}
	        	i++;
	        }
	        if (i == InvitedList.size())
	        {
	        	sendServerRpl(fd, ERR_INVITEONLYCHAN(users[fd].getUserNickName(), channel));
                continue;
	        }
		}
        
        // check if channel is full
        if (channels[channel].getChannelMembers().size() >= channels[channel].getChannelCap() && channels[channel].getChannelCap() != 0)
        {
            sendServerRpl(fd, ERR_CHANNELISFULL(users[fd].getUserNickName(), channel));
            continue;
        }

        // check if client is already in channel or not
        std::vector<User*> channelMembers = channels[channel].getChannelMembers();
        std::vector<User*>::const_iterator iti = channelMembers.begin();
        
        while (iti != channelMembers.end())
        {
            if (*(*iti) == users[fd])
            {
                break;
            }
            iti++;
        }

        // Add client to channel if not
        if (iti == channelMembers.end())
        {
            addClientToChannel(channels[channel], users[fd]);
        }
        // send error messsage if client already in channel
        else if (flag == 0)
		    std::cout << YELLOW << users[fd].getUserNickName() << "already in the channel\n" << RESET;
        sendChanInfo(channels[channel], users[fd]);
    }
}

void Server::sendChanInfo(Channel& channel, User& user)
{
    std::vector<User*>  channelMembers = channel.getChannelMembers();
    std::vector<User*>::const_iterator member = channelMembers.begin();
    while (member != channelMembers.end())
    {
        sendServerRpl((*member)->getUserSockId(), RPL_JOIN(user_id(user.getUserNickName(), user.getUserName()), channel.getName()));
        if ((*member) == &user)
        {
            sendServerRpl((*member)->getUserSockId(),RPL_TOPIC(user.getUserNickName(), channel.getName(), channel.getChannelTopic()));//rajouter topic
            sendServerRpl((*member)->getUserSockId(), RPL_NAMREPLY(user.getUserNickName(), channel.getSymbol(), channel.getName(), channel.listOfMember()));
            sendServerRpl((*member)->getUserSockId(), RPL_ENDOFNAMES(user.getUserNickName(), channel.getName()));
        }
        member++;
    }
}

bool atLeastOneAlphaNum(std::string toTest)
{
    for (size_t i = 0; i < toTest.size(); i++)
    {
        if(isalnum(toTest[i]) != 0)
            return(true);
    }
    return (false);
}

void addClientToChannel(Channel& channel, User& client)
{
    channel.addUser(client);
    client.addChannelList(channel);
}

std::string getChannelName(std::string strToPars)
{
    std::string toReturn;
    int i = 0;
    while (strToPars[i] && !isalnum(strToPars[i]) && strToPars[i] != '-' && strToPars[i] != '_')
        i++;
    while (strToPars[i] && strToPars[i] != ',')
    {
        toReturn += strToPars[i];
        i++;
    }
    return (toReturn);
    
}

std::string	getChannelKeyword(std::string strToPars)
{
    std::string toReturn;
    int i = 0;
    while (strToPars[i] && !isalnum(strToPars[i]) && strToPars[i] != '-' && strToPars[i] != '_')
        i++;
    while (strToPars[i] && strToPars[i] != ',')
    {
        toReturn += strToPars[i];
        i++;
    }
    return (toReturn);
}
