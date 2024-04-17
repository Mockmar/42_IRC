/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Notice.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nibenoit <nibenoit@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/22 14:39:00 by nwyseur           #+#    #+#             */
/*   Updated: 2023/11/30 13:57:29 by nibenoit         ###   ########.fr       */
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

void Server::broadcastToChannelNotice(std::string target, std::string speech, int fd)
{
    Channel broadcast = channels[target];
    std::vector<User*> banned_users = broadcast.getBannedUsers();
    for (std::vector<User *>::iterator it = banned_users.begin(); it != banned_users.end(); it++)
    {
        if (*it == &users[fd])
        {
            return; 
        }
    }

    std::vector<User*> kicked_users = broadcast.getKickedUsers();
    for (std::vector<User *>::iterator it = kicked_users.begin(); it != kicked_users.end(); it++)
    {
        if (*it == &users[fd])
        {
            return; 
        }
    }

    std::vector<User*> channel_users = broadcast.getChannelMembers();
    for (std::vector<User*>::iterator it = channel_users.begin(); it != channel_users.end(); it++)
    {
        std::string toSend = "#" + target;
        if ((*it)->getUserSockId() != fd)
            sendServerRpl((*it)->getUserSockId(), RPL_NOTICE(users[fd].getUserNickName(), users[fd].getUserName(), toSend, speech));
    }

}

void Server::notice(Message message, int fd)
{
    std::string target;
    std::string speech;

    if (message.getParameters().empty())
    {
        return;
    }
    else
        target = message.getParameters()[0];
    if (message.getTrailing().empty())
    {
        return;
    }
    else
    {
        speech = message.getTrailing();
        size_t posr = speech.find('\r');
        if (posr != std::string::npos)
            speech = speech.erase(posr, std::string::npos);
        size_t posn = speech.find('\n');
        if (posn != std::string::npos)
            speech = speech.erase(posn, std::string::npos);
    }
    
    if (target[0] == '#')
    {
        std::map<std::string, Channel>::iterator it;
        it = channels.find(target.substr(1));
        if (it == channels.end())
        {
            return;
        }
        else
        {
            broadcastToChannel(target.substr(1), speech, fd);
        }
    }
    else
    {
        std::map<int, User>::iterator itu;
        for (itu = users.begin(); itu != users.end(); ++itu)
        {
            if (itu->second.getUserName() == target || itu->second.getUserNickName() == target)
            {
                sendServerRpl(itu->second.getUserSockId(), RPL_NOTICE(users[fd].getUserNickName(), users[fd].getUserName(), target, speech));
                return;
            }
        }
        std::map<std::string, Channel>::iterator itc;
        itc = channels.find(target);
        if (itc != channels.end())
        {
            broadcastToChannel(target, speech, fd);
        }
        else
        {
            return;
        }
    }

}