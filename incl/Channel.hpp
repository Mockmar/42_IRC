/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nwyseur <nwyseur@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/06 17:10:17 by nwyseur           #+#    #+#             */
/*   Updated: 2023/12/01 14:32:41 by nwyseur          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

# include <string>
# include <iostream>
# include "Server.hpp"
# include "User.hpp"

class User;

class Channel
{
	public:
		Channel(void);
		Channel(const std::string& name, User& channelOperator);
		~Channel();

		// -- Add/Remove
		void addUser(User& user);
		void removeUser(User& user);
		void addBannedUser(User& user);
		void removeBannedUser(User& user);
		void addKickedUser(User& user);
		void removeKickedUser(User& user);

		void addInvitedUser(User& user);
		void removeInvitedUser(User& user);

		bool hasUser(User& user) const;
		bool isUserInMap(const std::map<int, User> users, const std::string userNickName);

		
		//mode
		bool is_operator(User& user) const;

		// -- Getter
		const std::vector<User*> getChannelMembers() const;
		const std::vector<User*> getBannedUsers() const;
		const std::vector<User*> getKickedUsers() const;
		const std::vector<User*> getChannelOperators() const;
		const std::vector<User*> getChannelInvitedUsers() const;
		const std::string getName() const;
		const std::string getPassword() const;
		size_t getChannelCap() const;
		const std::string getSymbol() const;
		const std::string getChannelTopic() const;
		const std::string getChannelMode() const;
		
		// -- Setter
		void	setPassword(std::string pass);
		void	setChannelCap(int cap);
		void	setChannelTopic(std::string topic);
		void	setChannelOperator(User& ops);

		void	setChannelMode(std::string mode);

		std::string listOfMember() const;
		
		bool hasOp(User& user) const;
		bool hasBanUser(User& user)const ;
		bool hasKickUser(User& user)const ;

		void addOperatorChannel(User& user);
		void removeChannelOperator(User& user);

		bool getInviteOnlyMode(void);
		void setInviteOnlyMode(bool mode);

		bool getTopicMode(void);
		void setTopicMode(bool mode);

		void add_mode_string(std::string mode);

	private:
		std::string _channelName;
		std::string _channelTopic;
		std::vector<User*>	_channelMembers;
		std::vector<User*>	_channelOperators;
		std::vector<User*>	_channelInvitedUser;
		std::vector<User*>	_bannedUsers; // ici + fonction
		std::vector<User*>	_kickedUsers; // ici + fonction
		std::string			_channelPassword;
		size_t				_channelCapacity;
		std::string			_channelSymbol;
		std::string			_channelModeString;
		
		bool				_inviteOnlyMode;
		bool				_topicMode;
		

};

#endif