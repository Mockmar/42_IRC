/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nibenoit <nibenoit@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:06:34 by nwyseur           #+#    #+#             */
/*   Updated: 2023/11/30 13:53:04 by nibenoit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(void)
{
	return;
}

Channel::Channel(const std::string &name,  User& channelOperator)
{
    _channelName = name;
    _channelOperators.push_back(&channelOperator);
    _channelMembers.push_back(&channelOperator);
	_channelInvitedUser.push_back(&channelOperator);
	_channelCapacity = 0;
	_channelSymbol = "#";
	_inviteOnlyMode = false;
}

Channel::~Channel()
{
	return;
}

// -- Getter
const std::vector<User*> Channel::getChannelMembers() const
{
	return (this->_channelMembers);
}

const std::vector<User*> Channel::getBannedUsers() const
{
	return (this->_bannedUsers);
}

const std::vector<User*> Channel::getKickedUsers() const
{
	return (this->_kickedUsers);
}

const std::vector<User*> Channel::getChannelOperators() const
{
	return (this->_channelOperators);
}

const std::vector<User*> Channel::getChannelInvitedUsers() const
{
	return (this->_channelInvitedUser);
}


const std::string Channel::getName() const
{
	return (this->_channelName);
}

const std::string Channel::getPassword() const
{
	return (this->_channelPassword);
}

size_t Channel::getChannelCap() const
{
	return (this->_channelCapacity);
}

const std::string Channel::getSymbol() const
{
	return (this->_channelSymbol);
}

const std::string Channel::getChannelTopic() const
{
	return (this->_channelTopic);
}

const std::string Channel::getChannelMode() const
{
	return (this->_channelModeString);
}

// -- Setter

void	Channel::setPassword(std::string pass)
{
	_channelPassword = pass;
}

void	Channel::setChannelCap(int cap)
{
	if (cap < 0)
		_channelCapacity = 0;
	else if (cap > 9999)
		_channelCapacity = 9999;
	else
		_channelCapacity = cap;
}

void	Channel::setChannelTopic(std::string topic)
{
	_channelTopic = topic;
}

void Channel::setChannelOperator(User& ops)
{
	_channelOperators.push_back(&ops);
}

// -- Add/Remove
void Channel::addUser(User& user)
{
    _channelMembers.push_back(&user);
}

void Channel::removeUser(User& user)
{
	unsigned long int i = 0;
	while (i < _channelMembers.size())
	{
		if (_channelMembers[i] == &user)
		{
			_channelMembers.erase(_channelMembers.begin() + i);
			return;
		}
		i++;
	}
}

void Channel::addInvitedUser(User& user)
{
    _channelInvitedUser.push_back(&user);
}

void Channel::removeInvitedUser(User& user)
{
	unsigned long int i = 0;
	while (i < _channelInvitedUser.size())
	{
		if (_channelInvitedUser[i] == &user)
		{
			_channelMembers.erase(_channelMembers.begin() + i);
			return;
		}
		i++;
	}
}

void Channel::addBannedUser(User& user)
{
	_bannedUsers.push_back(&user);
}

void Channel::removeBannedUser(User& user)
{
	unsigned long int i = 0;
	while (i < _bannedUsers.size())
	{
		if (_bannedUsers[i] == &user)
		{
			_bannedUsers.erase(_bannedUsers.begin() + i);
			return;
		}
		i++;
	}
}

void Channel::addKickedUser(User& user)
{
	_kickedUsers.push_back(&user);
}

void Channel::removeKickedUser(User& user)
{
	unsigned long int i = 0;
	while (i < _kickedUsers.size())
	{
		if (_kickedUsers[i] == &user)
		{
			_kickedUsers.erase(_kickedUsers.begin() + i);
			return;
		}
		i++;
	}
}

bool Channel::hasUser(User& user) const
{
	unsigned long int i = 0;
	while (i < _channelMembers.size())
	{
		if (_channelMembers[i] == &user)
		{
			return (true);
		}
		i++;
	}
	return (false);
}

bool Channel::hasOp(User& user) const
{
	unsigned long int i = 0;
	while (i < _channelOperators.size())
	{
		if (_channelOperators[i] == &user)
		{
			return (true);
		}
		i++;
	}
	return (false);
}

bool Channel::hasBanUser(User& user) const
{
	unsigned long int i = 0;
	while (i < _bannedUsers.size())
	{
		if (_bannedUsers[i] == &user)
		{
			return (true);
		}
		i++;
	}
	return (false);
}

bool Channel::hasKickUser(User& user) const
{
	unsigned long int i = 0;
	while (i < _kickedUsers.size())
	{
		if (_kickedUsers[i] == &user)
		{
			return (true);
		}
		i++;
	}
	return (false);
}

std::string Channel::listOfMember() const
{
	std::string toReturn;

	for (size_t i = 0; i < _channelMembers.size() - 1; i++)
	{
		if (is_operator(*_channelMembers[i]))
			toReturn += "@";
		toReturn += _channelMembers[i]->getUserNickName();
		if (i < _channelMembers.size() - 1)
		toReturn += " ";
	}

	return(toReturn);
}

//mode
bool Channel::is_operator(User& user) const
{
	unsigned long int i = 0;
	while (i < _channelOperators.size())
	{
		if (_channelOperators[i] == &user)
		{
			return (true);
		}
		i++;
	}
	return (false);
}

// La fonction pour vérifier si un utilisateur avec un certain surnom fait partie de la map
bool Channel::isUserInMap(std::map<int, User> users, std::string userNickName) {
    for (std::map<int, User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->second.getUserNickName() == userNickName) {
            // L'utilisateur avec le surnom spécifié a été trouvé dans la map
            return true;
        }
    }
    // L'utilisateur n'a pas été trouvé dans la map
    return false;
}

void Channel::addOperatorChannel(User& user)
{
	_channelOperators.push_back(&user);
	
}

void Channel::removeChannelOperator(User& user)
{
	unsigned long int i = 0;
	while (i < _channelOperators.size())
	{
		if (_channelOperators[i] == &user)
		{
			_channelOperators.erase(_channelOperators.begin() + i);
			return;
		}
		i++;
	}
}

void Channel::setChannelMode(std::string mode)
{
	_channelModeString = mode;
}


bool Channel::getInviteOnlyMode(void)
{
	return (_inviteOnlyMode);
}

void Channel::setInviteOnlyMode(bool mode)
{
	_inviteOnlyMode = mode;
}

void Channel::setTopicMode(bool mode)
{
	_topicMode = mode;
}

bool Channel::getTopicMode(void)
{
	return (_topicMode);
}

void Channel::add_mode_string(std::string mode)
{
	if (mode[0] == '+')
	{
		if (_channelModeString.find(mode[1]) == std::string::npos)
			_channelModeString += mode[1];
	}
	else if (mode[0] == '-')
	{
		if (_channelModeString.find(mode[1]) != std::string::npos)
			_channelModeString.erase(_channelModeString.find(mode[1]), 1);
	}
}

