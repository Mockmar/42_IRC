/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nwyseur <nwyseur@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 15:16:29 by nwyseur           #+#    #+#             */
/*   Updated: 2023/12/01 12:28:58 by nwyseur          ###   ########.fr       */
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
#include <string>
#include "Log.hpp"

std::vector<std::string> mySplit(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string extractNextWord(const std::string& input, const std::string& keyword) 
{
    std::istringstream iss(input);
    std::string word;

    // Recherche du mot-clé
    while (iss >> word) {
        if (word == keyword) {
            // Si le mot-clé est trouvé, renvoyer le prochain mot
            if (iss >> word) {
                return word;
            } else {
                // Gérer le cas où il n'y a pas de mot après le mot-clé
                return "Aucun mot après le mot-clé";
            }
        }
    }

    // Gérer le cas où le mot-clé n'est pas trouvé
    return "Mot-clé non trouvé";
}

void extractInfo(const std::string& input, User& user) 
{
    std::istringstream iss(input);
    std::string token;
    std::string value;

    while (iss >> token >> value) 
    {
        if (token == "NICK") 
        {
            user.setNickName(value);
        } 
        else if (token == "USER") 
        {
            user.setUserName(value);
        } 
        else if (token == "PASS") 
        {
            user.setPassword(value);
        }
    }
}


std::string TrimString(const std::string& str, const std::string& TrimStr) 
{
    size_t first = str.find_first_not_of(TrimStr);
    size_t last = str.find_last_not_of(TrimStr);

    if (first == std::string::npos || last == std::string::npos)
        return "";

    return str.substr(first, last - first + 1);
}

void TrimVectorWhiteSpace(std::vector<std::string> &vec, const std::string& TrimStr)
{
    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it) 
    {
        std::string trimmed = TrimString(*it, TrimStr);
        if (!trimmed.empty())
            *it = trimmed;
        else
            it = vec.erase(it);
    }
}

bool Server::userExistNameLeRetour(std::string user, int fd)
{
    std::map<int, User>::iterator it;
    for (it = users.begin(); it != users.end(); it++)
    {
        if (it->second.getUserNickName() == user && fd != it->first)
            return (true);
    }
    return (false);
}