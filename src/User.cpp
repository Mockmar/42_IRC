/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nwyseur <nwyseur@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/06 12:13:24 by nwyseur           #+#    #+#             */
/*   Updated: 2023/11/30 18:48:28 by nwyseur          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

User::User(void)
{
	return;
}

User::User(int sockId) : 
		_userSockId(sockId), _isConnected(false) 
{
	_userID = _nickName + "!" + _userName + "@localhost";
	return;
}

User::~User()
{
	return;
}

void User::setUserName(const std::string userName)
{
	_userName = userName;
	return;
}

void User::setOldNickName(const std::string userNickName)
{
	_oldNickName = userNickName;
	return;
}

void User::setNickName(const std::string userNickName)
{

	if (userNickName[userNickName.size() - 1] == '\n' && userNickName[userNickName.size() - 2] == '\r')
	{
		std::string substr =userNickName.substr(0, userNickName.size() - 2);
		_nickName = substr;
	}
	else
		_nickName = userNickName;
	return;
}

void User::setPassword(const std::string password)
{
	_password = password;
	return;
}

std::string User::getUserName(void)
{
	return (this->_userName);
}

std::string User::getUserNickName(void)
{
	return (this->_nickName);
}

std::string User::getOldNickName(void)
{
	return (this->_oldNickName);
}

std::string User::getPassword(void)
{
	return (this->_password);
}

void User::addChannelList(Channel& channel)
{
	_channelList.push_back(&channel);
	return;
}

void User::eraseChannelList(Channel& channel)
{
	for (std::vector<Channel*>::iterator it = _channelList.begin(); it != _channelList.end();)
    {
        if (*it == &channel)
            it = _channelList.erase(it);
        else
            ++it;
    }
}

int User::getUserSockId(void)
{
	return(this->_userSockId);
}

bool User::getIsConnected(void) const
{
	return _isConnected;
}

void User::setIsConnected(bool value)
{
	_isConnected = value;
}

bool User::isChannel(const std::string& name)
{
	for ( std::vector<Channel*>::iterator it = _channelList.begin(); it != _channelList.end();)
    {
   		if ((*it)->getName() == name)
        	return true;
        else
            ++it;
    }
    return false;
}
