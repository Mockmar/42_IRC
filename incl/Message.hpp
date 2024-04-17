/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jgautier <jgautier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/25 22:52:47 by nibenoit          #+#    #+#             */
/*   Updated: 2023/11/23 16:27:26 by jgautier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

class Message {
private:
	std::string rawMessage;
	std::string prefixe;
	std::string commande;
	std::vector<std::string> parameters;
	std::string	trailing;
	bool istrailing;

public:
	Message(const std::string& raw);
	~Message();

	std::string getPrefixe() const {
		return prefixe;
	}

	std::string getRawMessage() const {
		return rawMessage;
	}

	std::string getCommande() const {
		return commande;
	}

	std::string getTrailing() const {
		return trailing;
	}

	std::vector<std::string> getParameters() const {
		return parameters;
	}
	
};

#endif
