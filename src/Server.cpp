/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nibenoit <nibenoit@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/26 18:51:44 by nibenoit          #+#    #+#             */
/*   Updated: 2023/12/01 15:29:24 by nibenoit         ###   ########.fr       */
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
#include <csignal>

bool g_sig = true;

void quit(int)
{
    g_sig = false;
}


Server::Server(const std::string& port, const std::string& password) :
    _sockfd(-1), _nb_clients(0), _port(port),
    _password(password), _name(SERVER_NAME), _hints(), _servinfo(NULL)
{
    //setup date de creation
    time_t      rawtime = time(NULL);
    struct tm   *timeinfo;
    timeinfo = localtime(&rawtime);
	_date = std::string(asctime(timeinfo));

    // Affiche les informations sur le port et le mot de passe
    Log::info() << "Using port: " << port << '\n';
    Log::info() << "Using password: '" << password << "'" << '\n';

    // Initialise la structure _hints pour la fonction getaddrinfo
    memset(&_hints, 0, sizeof(_hints));
    _hints.ai_family = AF_INET;
    _hints.ai_socktype = SOCK_STREAM;
    _hints.ai_flags = AI_PASSIVE;

    // Obtient les détails de connexion du serveur en utilisant getaddrinfo
    if (getaddrinfo(NULL, _port.c_str(), &_hints, &_servinfo) != 0) {
        Log::error() << "Could not get server connection details" << '\n';
        exit(1);
    }
}

Server::~Server()
{
    freeaddrinfo(_servinfo);
    close(_sockfd);
    Log::info() << "Server stopped" << '\n';
}

int Server::start()
{
    // Création, liaison et écoute du socket du serveur
    signal(SIGINT, &quit);
    _sockfd = socket(_servinfo->ai_family, _servinfo->ai_socktype,
        _servinfo->ai_protocol);
    if (_sockfd == -1) {
        Log::error() << "Could not create server socket" << '\n';
        return (1);
    }
    int optvalue = 1; // enables the re-use of a port if the IP address is different
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue)) < 0)
    {
        Log::error() << "Error with setsockopt" << std::endl;
        return (1);
    }
    if (bind(_sockfd, _servinfo->ai_addr, _servinfo->ai_addrlen) == -1) {
        Log::error() << "Could not bind server socket" << '\n';
        return (1);
    }
    if (listen(_sockfd, MAX_CONNEXIONS) == -1) {
        Log::error() << "Could not listen on server socket" << '\n';
        return (1);
    }
    Log::info() << "Server started" << '\n';

    return (0);
}

int Server::poll()
{
    // Crée une structure pour le socket du serveur
    epoll_event server_event;
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd == -1) {
        Log::error() << "Could not create epoll fd" << '\n';
        g_sig = false;
    }
    server_event.events = EPOLLIN;
    server_event.data.fd = _sockfd;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _sockfd, &server_event) == -1) {
        Log::error() << "Could not add server fd to epoll" << '\n';
        g_sig = false;
    }

    while (g_sig) {
        // Ne pas réutiliser num_events, car vous avez déjà une variable i dans la boucle for
        int num_events = epoll_wait(_epoll_fd, _events, MAX_CONNEXIONS, 0);
        if (num_events == -1) 
        {
            break;
        }

        for (int i = 0; i < num_events; i++) {
            int fd = _events[i].data.fd;
            if (fd == _sockfd) {
                create_client();
            } else {
                receive_message(fd);
            }
        }
    }
    close(_epoll_fd);
    return 0;
}

int Server::create_client()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int client_fd = accept(_sockfd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (_nb_clients < MAX_CONNEXIONS)
    {

        if (client_fd == -1)
        {
            Log::error() << "Could not accept the client connection" << '\n';
            exit(1);
        }

        // Vous devez créer une nouvelle structure epoll_event pour le nouveau client.
        struct epoll_event client_event;
        client_event.data.fd = client_fd;
        client_event.events = EPOLLIN;

        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
        {
            perror("epoll_ctl");
            exit(1);
        }

        ++_nb_clients;
        sleep(1);
        Log::info() << "Client connected : " << client_fd << '\n';
        addUser(client_fd);
    }
    else
    {
        Log::info() << "Max clients reached" << '\n';
        sendServerRpl(client_fd, "ERROR :Max clients reached\r\n");
        close(client_fd);
        
    }
    return 0;
}

int Server::WrongPassWord(std::string str, int fd)
{
    if (extractNextWord(str, "PASS") == "Mot-clé non trouvé")
        sendServerRpl(fd, ERR_NEEDMOREPARAMS(extractNextWord(str, "NICK"), "PASS"));
    else
        sendServerRpl(fd, ERR_PASSWDMISMATCH(extractNextWord(str, "NICK")));
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        perror("epoll_ctl");
        exit(1);
    }
    users.erase(fd);
    close(fd);
    --_nb_clients;
    Log::info() << "Client disconnected" << '\n';
    return 1;
}

int Server::receive_message(int fd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    std::string action;

    // Lire les données du socket
    int num_bytes = recv(fd, buffer, sizeof(buffer), 0);
    if (num_bytes == -1)
    {
        perror("recv");
        exit(1);
    }
    if (num_bytes == 0)
    {
        Message message("QUIT :crash");
        serverquit(message, fd);
        return (0);
    }
    std::string str(buffer);
    if (!str.empty())
        std::cout << "Received << " << str << std::endl;
    if (str.find("\r\n") == std::string::npos)
    {
        _ctrlDBuff.push_back(str);
        return (1);
    }
    else
    {
        for ( size_t i = 0; i < _ctrlDBuff.size(); i++)
        {
            action += _ctrlDBuff[i];
        }
        action += buffer;
    }
    if (users[fd].getIsConnected() == false)
    {
        extractInfo(action, users[fd]);
        if (!users[fd].getPassword().empty())
        {
             if (users[fd].getPassword() != _password)
                return (WrongPassWord(action,fd));
         }
        if (userHasAllInfo(fd))
        {
            users[fd].setIsConnected(true);
            if (userExistNameLeRetour(users[fd].getUserNickName(), fd))
                users[fd].setNickName(changeNickname(users[fd].getUserNickName(), users[fd].getUserName(), fd));
            // Envoi du code RPL au client
            sendServerRpl(fd, RPL_WELCOME(user_id(users[fd].getUserNickName(), users[fd].getUserName()), users[fd].getUserNickName()));
            sendServerRpl(fd, RPL_YOURHOST(users[fd].getUserNickName(), SERVER_NAME, SERVER_VERSION));
            sendServerRpl(fd, RPL_CREATED(users[fd].getUserNickName(), this->_date));
            sendServerRpl(fd, RPL_MYINFO(users[fd].getUserNickName(), SERVER_NAME, SERVER_VERSION, "io", "kost", "k"));
            sendServerRpl(fd, RPL_ISUPPORT(users[fd].getUserNickName(), "CHANNELLEN=50 NICKLEN=30 TOPICLEN=307"));
         }
    }
    else
    {
        executeCommand(action, fd);
    }
    _ctrlDBuff.clear();


    return 0;
}

int Server::executeCommand(std::string str, int fd)
{
    Message message(str);
    if (message.getCommande() == "NICK")
        setUserNickName(message, fd);
    else if (message.getCommande() == "PRIVMSG")
        sendPrivateMessage(message, fd);
    else if (message.getCommande() == "JOIN")
        executeJoinOrder(message, fd);
    else if (message.getCommande() == "PART")
        executePart(message, fd);
    else if (message.getCommande() == "PING")
        sendPong(message, fd);
    else if (message.getCommande() == "KICK")
        executeKick(message, fd);
    else if (message.getCommande() == "TOPIC")
        setReadTopic(message, fd);
    else if (message.getCommande() == "INVITE")
        sendInvitation(message, fd);
    else if (message.getCommande() == "NOTICE")
        notice(message, fd);
	else if (message.getCommande() == "MODE")
		handleMode(message, fd);
    else if (message.getCommande() == "QUIT")
		serverquit(message, fd);
    else
        std::cout << "-------" << std::endl;
    return (0);
}

void Server::addUser(int sockId)
{
    users.insert(std::make_pair(sockId, User(sockId)));
    return;
}

void Server::addChannel(const std::string& name, User& channelOperator)
{
    channels.insert(std::make_pair(name, Channel(name, channelOperator)));
    
    return;
}

bool Server::isChannel(const std::string& name)
{
    std::map<std::string, Channel>::iterator it = channels.find(name);
    if (it != channels.end()) 
        return true;
    return false;
}

void Server::sendServerRpl(int const fd, std::string reply)
{
	std::istringstream	buf(reply);
	std::string			sended;
	
	send(fd, reply.c_str(), reply.size(), 0);
	while (getline(buf, sended))
	{
		std::cout << "[Server] Message sent to client " << fd << "       >> " << GREEN << sended << RESET << std::endl;
	}
}

bool Server::is_valid_port(const std::string& port)
{
    if (port.empty())
        return false;
    for (std::string::const_iterator it = port.begin(); it != port.end(); it++) {
        if (!isdigit(*it))
            return false;
    }
    int portNum = atoi(port.c_str());
    return portNum >= 1024 && portNum <= 65535;
}

bool Server::is_valid_password(const std::string& password)
{
    if (password.empty() || password.size() > 255)
        return false;
    for (std::string::const_iterator it = password.begin(); it != password.end(); it++) {
        if (isspace(*it) || *it == '\0' || *it == ':')
            return false;
    }
    return true;
}

int Server::userNameToFd(std::string& user)
{
    std::map<int, User>::iterator it;
    for (it = users.begin(); it != users.end(); it++)
    {
        if (it->second.getUserNickName() == user)
            return (it->first);
    }
    return (-1);
}


std::string Server::getDate() const
{
    return (this->_date);
}

int Server::getUserIdByNickName(std::string& userNickName) {
    for (std::map<int, User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->second.getUserNickName() == userNickName) {
            return it->first; // Retourne l'ID de l'utilisateur trouvé
        }
    }
    return -1; // Retourne -1 si l'utilisateur n'est pas trouvé
}

bool Server::userHasAllInfo(int fd)
{
    if (users[fd].getUserNickName().empty() || users[fd].getUserName().empty() || users[fd].getPassword().empty()) 
        return false;
    else
        return true;
}