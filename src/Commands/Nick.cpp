/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nwyseur <nwyseur@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 15:12:23 by nwyseur           #+#    #+#             */
/*   Updated: 2023/11/14 12:46:37 by nwyseur          ###   ########.fr       */
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

void Server::setUserNickName(Message message, int fd)
{
    if (message.getParameters()[0] == "")
    {
        sendServerRpl(fd, ERR_NONICKNAMEGIVEN(users[fd].getUserNickName()));
        return;
    }
    for (unsigned long int i = 0; i < message.getParameters()[0].size(); i++)
    {
        if (isalnum(message.getParameters()[0][i]) == 0 && (message.getParameters()[0][i] < 91 && message.getParameters()[0][i] > 95) && message.getParameters()[0][i] != 45 
            && message.getParameters()[0][i] != 123 && message.getParameters()[0][i] != 125)
        {
            sendServerRpl(fd, ERR_ERRONEUSNICKNAME(users[fd].getUserNickName(), message.getParameters()[0]));
            return;
        }
    }
    std::map<int, User>::iterator it;
    for (it = users.begin(); it != users.end(); ++it)
    {
        if (it->second.getUserNickName() == message.getParameters()[0])
        {
            sendServerRpl(fd, ERR_NICKNAMEINUSE(users[fd].getUserNickName(), message.getParameters()[0]));
            return;
        }
    }
    users[fd].setOldNickName(users[fd].getUserNickName());
    users[fd].setNickName(message.getParameters()[0]);
    std::cout << GREEN << "Nickname of user " << users[fd].getUserSockId() << " updated to -> " << users[fd].getUserNickName() << RESET << std::endl;
    sendServerRpl(fd, RPL_NICK(users[fd].getOldNickName(), users[fd].getUserName(), users[fd].getUserNickName()));
}