#ifndef _USER_HPP
# define _USER_HPP

# include <string>
# include <iostream>
# include <deque>
# include <sys/socket.h>
# include <netinet/in.h>

# include "Server.hpp"
# include "Channel.hpp"

#define RPL_WHOISUSER(requestingUserNick, inquiredUserNick, id, realHost, realName)	((std::string)SERVER_NAME + "311 " + requestingUserNick + " " + inquiredUserNick + " " + id + " " + realHost + " * :" + realName + "\r\n");
#define RPL_WHOISSERVER(requestingUserNick, inquiredUserNick)						((std::string)SERVER_NAME + "312 " + requestingUserNick + " " + inquiredUserNick + " " + SERVER_NAME + ":" + SERVER_DESCRIPTION + "\r\n");
#define RPL_WHOISCHANNELS(requestingUserNick, inquiredUserNick, channelList)		((std::string)SERVER_NAME + "319 " + requestingUserNick + " " + inquiredUserNick + " " + ":" + channelList + "\r\n");
#define RPL_ENDOFWHOIS(requestingUserNick, inquiredUserNick)						((std::string)SERVER_NAME + "318 " + requestingUserNick + " " + inquiredUserNick + " " + ":End of WHOIS list\r\n");

class Server;
class Channel;

class User {

public:

	//CONSTRUCTORS, DESTRUCTORS & OPERATORS
	User(void);
	User(std::string const & username, std::string const & nickname);
	~User(void);

	bool	operator==(const User &user) const;

	// SETTERS,  GETTERS AND CHECKERS	
	void	setUsername(std::string const &username);
	void	setNickname(std::string const &nickname);
	void	setInet(std::string const &inet);
	void	setCommands(std::deque<std::string> const &commands);
	void	setBuffer(std::string const &buffer);
	void	setSocket(int const &socket);
	void	setAddr(sockaddr_in const &addr);
	void	setStatus(bool const &connected);
	void	setSent(bool const &connected);
	
	const std::string& 				getUsername() const;
	const std::string&				getNickname() const;
	const std::string&				getInet() const;
	const std::string&				getBuffer() const;
	const int&						getSocket() const;
	const struct sockaddr_in&		getAddr() const;
	const std::deque<std::string>&	getCommands() const;
	const std::string&				getSender() const;
	const std::string				getChannelJoinedNames() const;
	const std::vector<Channel *>	&getChannelJoined() const;

	int								createUserSocket(int socketServer);

	void							quit(Server  & server);

	const bool&						isConnected() const;
	const bool&						isSent() const;
	void							displayInfo() const;

	void							updateSender();

	//COMMANDS MANAGEMENT
	void	addCommand(std::string const & command);
	void	addChannel(Channel *channel);
	void	leaveChannel(Channel *channel);

	void	whoIs(Server const &server, User &requestingUser) const;

private:
	std::string 			_username;
	std::string 			_nickname;
	
	std::string 			_commandBuffer;

	int 					_socket;
	struct sockaddr_in		_addr;
	std::string				_inetNtoa;
	std::string				_sender;

	bool					_isConnected;

	std::deque<std::string>	_commands;
	std::vector<Channel *>	_channelsJoined;

	bool					_connectionSent;
};

#endif