/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nibenoit <nibenoit@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 15:12:23 by nwyseur           #+#    #+#             */
/*   Updated: 2023/11/30 13:59:15 by nibenoit         ###   ########.fr       */
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

void Server::broadcastToChannel(std::string target, std::string speech, int fd)
{
    Channel broadcast = channels[target];
    std::vector<User*> banned_users = broadcast.getBannedUsers();
    for (std::vector<User *>::iterator it = banned_users.begin(); it != banned_users.end(); it++)
    {
        if (*it == &users[fd])
        {
            std::cout << RED << "[Server] User " << users[fd].getUserNickName() << " is banned from channel " << RESET << broadcast.getName();
            return; 
        }
    }

    std::vector<User*> kicked_users = broadcast.getKickedUsers();
    for (std::vector<User *>::iterator it = kicked_users.begin(); it != kicked_users.end(); it++)
    {
        if (*it == &users[fd])
        {
            std::cout << RED << "[Server] User " << users[fd].getUserNickName() << " is kicked from channel " << RESET << broadcast.getName();
            return; 
        }
    }

    std::vector<User*> channel_users = broadcast.getChannelMembers();
    for (std::vector<User*>::iterator it = channel_users.begin(); it != channel_users.end(); it++)
    {
        std::string toSend = "#" + target;
        if ((*it)->getUserSockId() != fd)
            sendServerRpl((*it)->getUserSockId(), RPL_PRIVMSG(users[fd].getUserNickName(), users[fd].getUserName(), toSend, speech));
    }

}

void Server::sendPrivateMessage(Message message, int fd)
{
    std::string target;
    std::string speech;

    if (message.getParameters().empty())
    {
        sendServerRpl(fd, ERR_NORECIPIENT(users[fd].getUserNickName()));
        return;
    }
    else
        target = message.getParameters()[0];
    if (message.getTrailing().empty())
    {
        sendServerRpl(fd, ERR_NOTEXTTOSEND(users[fd].getUserNickName()));
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
            sendServerRpl(fd, ERR_NOSUCHNICK(users[fd].getUserNickName(), target));
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
                sendServerRpl(itu->second.getUserSockId(), RPL_PRIVMSG(users[fd].getUserNickName(), users[fd].getUserName(), target, speech));
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
            sendServerRpl(fd, ERR_NOSUCHNICK(users[fd].getUserNickName(), target));
            return;
        }
    }

}






//void Server::sendPrivateMessage(Message message, int fd)
//{
//    std::string sender = users[fd].getUserNickName();
//    sender.resize(sender.size() - 2);
//
//    std::map<int, User>::iterator it;
//    for (it = users.begin(); it != users.end(); ++it)
//    {
//        if (it->second.getUserName() == message.getParameters()[0] + "\r\n" || it->second.getUserNickName() == message.getParameters()[0] + "\r\n")
//        {
//            std::string buffer = "[" + sender + "]";
//            for (unsigned long int k = 2; k < message.getParameters().size(); k++)
//                buffer = buffer + " " + message.getParameters()[k];
//            send(it->second.getUserSockId(), buffer.c_str(), buffer.size(), 0);
//            return;
//        }
//    }
//    std::string error = "Error: the user you're trying to reach doesn't exist\r\n";
//    send(fd, error.c_str(), error.size(), 0);
//}
 