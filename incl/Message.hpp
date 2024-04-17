
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
