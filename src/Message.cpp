/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jgautier <jgautier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 18:00:45 by nwyseur           #+#    #+#             */
/*   Updated: 2023/11/27 16:22:57 by jgautier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"
#include "Server.hpp"

Message::Message(const std::string& raw) : rawMessage(raw), istrailing(false)
{
    istrailing = false;
    std::istringstream stream(TrimString(rawMessage, "\n\r"));
    std::istringstream tmp(stream.str());
	
	if (rawMessage.empty())
		return ;
    if (rawMessage[0] == ':') 
	{
        std::getline(stream, prefixe, ' ');
        std::getline(tmp, prefixe, ' ');
        prefixe = prefixe.substr(1);
    }
    std::getline(stream, commande, ' ');
    std::getline(tmp, commande, ' ');
    std::string parametre;
    while (std::getline(stream, parametre, ' ')) 
	{
		if (parametre[0] == ':')
		{
            istrailing = true;
			std::getline(tmp, trailing, '\n');
        	trailing = trailing.substr(1);
			break ;
		}
        parameters.push_back(parametre);
		std::getline(tmp, parametre, ' ');
    }
}

Message::~Message()
{

}