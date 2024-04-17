#include "Server.hpp"
#include <string>


void Server::handleMode(Message message, int fd)
{
	User& user = users[fd];
	
	// lors de la premiere connexion : getParameters()[0] = user.getUserNickName() et non pas le channel
	if (message.getParameters()[0] == user.getUserNickName())
	{
		// ne rien faire
		return ;
	}

	
	std::string command = message.getCommande();
	std::cout << "User " << user.getUserNickName() << " is trying to execute command " << command << std::endl;

	if (command.empty())
	{
		sendServerRpl(fd, ERR_UNKNOWNCOMMAND(user.getUserNickName(), command));
		return ;
	}

	if (message.getParameters().empty())
	{
		Log::warning() << "User " << user.getUserNickName() << " is trying to execute command " << command << " without target" << std::endl;
		sendServerRpl(fd, ERR_NEEDMOREPARAMS(user.getUserNickName(), command));
		return ;
	}

	// ---------------------------------------------- //
	std::string channel = message.getParameters()[0];
	// Log::info() << "channel before = " << channel << std::endl;
	//channel ne peut que commencer par #
	if (channel[0] != '#')
	{
		Log::warning() << "User " << user.getUserNickName() << " is trying to execute command " << command << " with an invalid channel name" << std::endl;
		sendServerRpl(fd, ERR_NOSUCHCHANNEL(user.getUserNickName(), channel));
		return ;
	}

    size_t posr = channel.find('\r');//ici
    if (posr != std::string::npos)
    {
        channel = channel.erase(posr, std::string::npos);
    }
    size_t posn = channel.find('\n');
    if (posn != std::string::npos)
    {
        channel = channel.erase(posn, std::string::npos);
    }
	if (channel.empty() || channel.find('#') == channel.npos)
    {
        sendServerRpl(fd, ERR_NEEDMOREPARAMS(users[fd].getUserNickName(), message.getCommande()));
        return;
    }
    channel.erase(channel.find("#"), 1);
	Log::info() << "channel name = " << channel << std::endl;

	Channel& target = channels[channel];
	if (target.getName().empty())
	{
		Log::warning() << "User " << user.getUserNickName() << " is trying to execute command " << command << " on an unknown channel" << std::endl;
		sendServerRpl(fd, ERR_NOSUCHCHANNEL(user.getUserNickName(), message.getParameters()[0]));
		return ;
	}

	if (message.getParameters().size() == 1)
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command " << command << " on channel " << target.getName() << std::endl;
		sendServerRpl(fd, RPL_CHANNELMODEIS(user.getUserNickName(), target.getName(), target.getChannelMode()));
		return ;
	}

	std::string mode = message.getParameters()[1];
	Log::info() << "mode = " << mode << std::endl;
	if (mode[0] == 'b')
	{
		return ;
	}

	if (mode[0] != '+' && mode[0] != '-')
	{
		Log::warning() << "User " << user.getUserNickName() << " is trying to execute command " << command << " with an invalid mode + OU - INEXISTANT" << std::endl;
		sendServerRpl(fd, ERR_UNKNOWNMODE(user.getUserNickName(), mode));
		return ;
	}

	posr = mode.find('\r');
    if (posr != std::string::npos)
    {
        mode = mode.erase(posr, std::string::npos);
    }
    posn = mode.find('\n');
    if (posn != std::string::npos)
    {
        mode = mode.erase(posn, std::string::npos);
    }

	if (!target.is_operator(user))
	{
		Log::warning() << "User " << user.getUserNickName() << " is trying to execute command " << command << " on channel " << target.getName() << " but is not operator" << std::endl;
		sendServerRpl(fd, ERR_CHANOPRIVSNEEDED(user.getUserNickName(), target.getName()));
		return ;
	}

	bool operand = 1; // +o = 1, -o = 0
	//mode = +o

	for (std::string::const_iterator it = mode.begin(); it != mode.end(); ++it)
	{
		if (*it == '+') // +o
			operand = 1;
		else if (*it == '-') // -o
			operand = 0;
		else if (*it == 'o')
			operator_mode(target, user, operand, message);
		else if (*it == 'i')
			invite_only_mode(target, user, operand, message);
		else if (*it == 't')
			topic_mode(target, user, operand, message);
		else if (*it == 'k')
			password_mode(target, user, operand, message);
		else if (*it == 'l')
			limit_mode(target, user, operand, message);
		else
		{
			Log::warning() << "User " << user.getUserNickName() << " is trying to execute command " << command << " with an invalid mode" << std::endl;
			sendServerRpl(fd, ERR_UNKNOWNMODE(user.getUserNickName(), mode));
			return ;
		}
	}
	return ;
	
}

void Server::limit_mode(Channel& channel, User& user, bool operand, Message& message)
{
	Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;

	if (operand)
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;
		
		if (message.getParameters().size() < 3)
		{
			Log::warning() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " but without limit" << std::endl;
			sendServerRpl(user.getUserSockId(), ERR_NEEDMOREPARAMS(user.getUserNickName(), message.getCommande()));
			return ;
		}

		std::string limit = message.getParameters()[2];
		if (limit.empty())
		{
			Log::warning() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " but without limit" << std::endl;
			sendServerRpl(user.getUserSockId(), ERR_NEEDMOREPARAMS(user.getUserNickName(), message.getCommande()));
			return ;
		}

		channel.setChannelCap(std::atoi(limit.c_str()));

		std::string reply = ":" + user.getUserNickName() + " MODE #" + channel.getName() + " +l " + limit + "\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), channel.getName(), "+l"));
		channel.add_mode_string("+l");

		// //afficher les modes
		Log::info() << "channel mode string = " << channel.getChannelMode() << std::endl;
		return ;

	}
	else
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;
		channel.setChannelCap(0);
		std::string reply = ":" + user.getUserNickName() + " MODE #" + channel.getName() + " -l\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), channel.getName(), "-l"));

		channel.add_mode_string("-l");

		//afficher les modes
		Log::info() << "channel mode string = " << channel.getChannelMode() << std::endl;
		return ;
	}
}

void Server::password_mode(Channel& channel, User& user, bool operand, Message& message)
{
	Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;

	if (operand)
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;
		
		if (message.getParameters().size() < 3)
		{
			Log::warning() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " but without password" << std::endl;
			sendServerRpl(user.getUserSockId(), ERR_NEEDMOREPARAMS(user.getUserNickName(), message.getCommande()));
			return ;
		}

		std::string password = message.getParameters()[2];
		if (password.empty())
		{
			Log::warning() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " but without password" << std::endl;
			sendServerRpl(user.getUserSockId(), ERR_NEEDMOREPARAMS(user.getUserNickName(), message.getCommande()));
			return ;
		}

		channel.setPassword(password);
		std::string reply = ":" + user.getUserNickName() + " MODE #" + channel.getName() + " +k " + password + "\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), channel.getName(), "+k"));
		channel.add_mode_string("+k");

		// //afficher les modes
		Log::info() << "channel mode string = " << channel.getChannelMode() << std::endl;
		return ;

	}
	else
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;
		channel.setPassword("");
		std::string reply = ":" + user.getUserNickName() + " MODE #" + channel.getName() + " -k\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), channel.getName(), "-k"));

		channel.add_mode_string("-k");

		//afficher les modes
		Log::info() << "channel mode string = " << channel.getChannelMode() << std::endl;
		return ;
	}
}

void Server::topic_mode(Channel& channel, User& user, bool operand, Message& message)
{
	(void)message;
	Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;

	if (operand)
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;
		channel.setTopicMode(true);
		std::string reply = ":" + user.getUserNickName() + " MODE #" + channel.getName() + " +t\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), channel.getName(), "+t"));
		channel.add_mode_string("+t");
		
		//afficher les modes
		Log::info() << "channel mode string = " << channel.getChannelMode() << std::endl;
		return ;
	}
	else
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;
		channel.setTopicMode(false);
		std::string reply = ":" + user.getUserNickName() + " MODE #" + channel.getName() + " -t\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), channel.getName(), "-t"));
		channel.add_mode_string("-t");

		//afficher les modes
		Log::info() << "channel mode string = " << channel.getChannelMode() << std::endl;
		return ;
	}
}


void Server::invite_only_mode(Channel& channel, User& user, bool operand, Message& message)
{
	(void)message;
	Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;

	if (operand)
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;
		channel.setInviteOnlyMode(true);
		std::string reply = ":" + user.getUserNickName() + " MODE #" + channel.getName() + " +i\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), channel.getName(), "+i"));
		channel.add_mode_string("+i");

		//afficher les modes
		Log::info() << "channel mode string = " << channel.getChannelMode() << std::endl;
		return ;
	}
	else
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;
		channel.setInviteOnlyMode(false);
		std::string reply = ":" + user.getUserNickName() + " MODE #" + channel.getName() + " -i\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), channel.getName(), "-i"));
		channel.add_mode_string("-i");

		//afficher les modes
		Log::info() << "channel mode string = " << channel.getChannelMode() << std::endl;
		return ;
	}
}

void Server::operator_mode(Channel& channel, User& user, bool operand, Message& message)
{
	Log::info() << "Operand = " << operand << std::endl;
	Log::info() << "les operateurs du channel : ";
		std::vector<User*> operators = channel.getChannelOperators();
		for (std::vector<User*>::const_iterator it = operators.begin(); it != operators.end(); ++it)
		{
			std::cout << (*it)->getUserNickName() << " ";
		}
		std::cout << std::endl;
	Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << std::endl;



	// Vérifiez que message.getParameters() a au moins 3 éléments avant d'accéder à l'index 2
    if (message.getParameters().size() < 3)
    {
        Log::warning() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " but without target user" << std::endl;
        sendServerRpl(user.getUserSockId(), ERR_NEEDMOREPARAMS(user.getUserNickName(), message.getCommande()));
        return ;
    }

	std::string targetUserNickName = message.getParameters()[2];
	Log::info() << "targetUserNickName = " << targetUserNickName << std::endl;
	if (targetUserNickName.empty())
	{
		Log::warning() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " but without target user" << std::endl;
		sendServerRpl(user.getUserSockId(), ERR_NEEDMOREPARAMS(user.getUserNickName(), message.getCommande()));
		return ;
	}

	// TargetUser n'est pas dans le channel
	else if (!channel.isUserInMap(users, targetUserNickName))
	{
		Log::warning() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " but target user " << targetUserNickName << " is not in channel" << std::endl;
		sendServerRpl(user.getUserSockId(), ERR_USERNOTINCHANNEL(user.getUserNickName(), targetUserNickName, channel.getName()));
		return ;
	}

	// Operand = "+" et TargetUser n'est pas operateur
	else if (operand && !channel.is_operator(users[getUserIdByNickName(targetUserNickName)]))
	{
		
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " and target user " << targetUserNickName << std::endl;
		channel.addOperatorChannel(users[getUserIdByNickName(targetUserNickName)]);
		channel.addInvitedUser(users[getUserIdByNickName(targetUserNickName)]);
		std::string reply = ":" + user.getUserNickName() + " add " + targetUserNickName + " as operator\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), targetUserNickName, "+o"));
		return ;
	}

	// Operand = "+" et TargetUser est deja un operateur
	else if (operand && channel.is_operator(users[getUserIdByNickName(targetUserNickName)]))
	{
		Log::warning() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " and target user " << targetUserNickName << " but target user is already operator" << std::endl;
		return ;
	}

	// Operand = "-" et TargetUser est operateur
	else if (!operand && channel.is_operator(users[getUserIdByNickName(targetUserNickName)]))
	{
		Log::info() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " and target user " << targetUserNickName << std::endl;
		channel.removeChannelOperator(users[getUserIdByNickName(targetUserNickName)]);
		std::string reply = ":" + user.getUserNickName() + " MODE #" + channel.getName() + " -o " + targetUserNickName + "\r\n";
		broadcastToChannel(channel.getName() , reply, user.getUserSockId());
		sendServerRpl(user.getUserSockId(), RPL_UMODEIS(user.getUserNickName(), targetUserNickName, "-o"));
		return ;
	}

	else
	{
		//unknown command
		Log::warning() << "User " << user.getUserNickName() << " is trying to execute command MODE on channel #" << channel.getName() << " with operand " << operand << " and target user " << targetUserNickName << " but unknown command" << std::endl;
		sendServerRpl(user.getUserSockId(), ERR_UNKNOWNCOMMAND(user.getUserNickName(), message.getCommande()));
		return ;
	}
	
}

