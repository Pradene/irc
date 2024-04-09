#include "Server.hpp"



// CONNECTION PART

bool	Server::userIsConnected(User const &user) {
	return (!user.getUsername().empty() \
	&& !user.getNickname().empty() \
	&& user.isConnected());
}


void	Server::checkConnection(User &user) {
	if (userIsConnected(user) && !user.isSent()) {
		welcome(user);
		user.setSent(true);
	}
}


bool	Server::password(User &user, std::vector<std::string> const &args) {
	if (user.isConnected())
		return (true);

	std::string	mess;

	mess = CMD_NOTICE_TARGET(SERVER_NAME, std::string("*** Checking your password..."), std::string("AUTH"));
	sendMessageToUser(user, mess);
	
	if (args.size() != 1 || !checkPassword(args[0])) {
		mess = CMD_NOTICE_TARGET(SERVER_NAME, std::string("*** Wrong password, please try again..."), std::string("AUTH"));
		sendMessageToUser(user, mess);
		removeUser(user);
		return (false);

	} else {
		mess = CMD_NOTICE_TARGET(SERVER_NAME, std::string("*** Password is correct..."), std::string("AUTH"));
		sendMessageToUser(user, mess);
		user.setStatus(true);
		checkConnection(user);
		return (true);
	}
}


void	Server::userName(User &user, std::vector<std::string> const &args)
{
	std::string	mess;
	mess = CMD_NOTICE_TARGET(SERVER_NAME, std::string("*** Checking your username..."), std::string("AUTH"));
	sendMessageToUser(user, mess);

	if (args.size() < 1) {
		mess = CMD_NOTICE_TARGET(SERVER_NAME, std::string("*** Your username is empty..."), std::string("AUTH"));
		sendMessageToUser(user, mess);
		return ;

	} else {
		user.setUsername(args[0]);
		mess = CMD_NOTICE_TARGET(SERVER_NAME, std::string("*** Username successfully set..."), std::string("AUTH"));
		sendMessageToUser(user, mess);
		checkConnection(user);
	}
}


void	Server::nickName(User &user, std::vector<std::string> const &args)
{
	std::string	mess;
	mess = CMD_NOTICE_TARGET(SERVER_NAME, std::string("*** Checking your nickname..."), std::string("AUTH"));
	if (args.size() == 0) {
		mess = CMD_NOTICE_TARGET(SERVER_NAME, std::string("*** Your nickname is empty..."), std::string("AUTH"));
		sendMessageToUser(user, mess);
		return ;
	}

	try {
		if (findUserByNickname(args[0])) {

			if (!user.isSent()) {
				std::string	err = ERR_NICKNAMEINUSE("*", args[0]);
				throw std::runtime_error(err);
			} else {
				std::string	err = ERR_NICKNAMEINUSE(user.getNickname(), args[0]);
				throw std::runtime_error(err);
			}
		} else {
			if (!user.isSent()) {
				user.setNickname(args[0]);
				mess = CMD_NICK(user.getSender(), args[0]);
				sendMessageToUser(user, mess);
			} else {
				mess = CMD_NICK(user.getSender(), args[0]);
				sendMessageToUser(user, mess);
				user.setNickname(args[0]);
			}
		}

		mess = CMD_NOTICE_TARGET(SERVER_NAME, std::string("*** Nickname successfully set..."), std::string("AUTH"));
		sendMessageToUser(user, mess);

		checkConnection(user);

	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		sendMessageToUser(user, e.what());
	}
}

void	Server::quit(User &user, std::string const &reason)
{
	std::string 					mess = CMD_QUIT(user.getSender(), reason);
	struct epoll_event				ev;
	std::vector<User *>::iterator	userIt;
	std::vector<Channel *>			channels;
	
	if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, user.getSocket(), &ev) == -1)
		return ;

	channels = user.getChannelJoined();
	for (std::vector<Channel *>::iterator channelIt = channels.begin(); channelIt != channels.end(); ++channelIt) {
		(*channelIt)->removeUser(user);
		sendMessageToALL(user, (*channelIt)->getMembers(), mess);
	}
	
	userIt = std::find(_users.begin(), _users.end(), &user);
	if (userIt == _users.end())
		return ;
	
	_users.erase(userIt);
	delete &user;
}



// PONG

/*
Sends a formatted "PONG" response to a user,
typically used to respond to a "PING" message from the IRC server to keep the connection active.
*/
void    Server::pong(const User &user, std::vector<std::string> const & args) const
{
	if (args.size() < 1)
		return;
	std::string const & message = args[0];
    std::string mess = CMD_PING(message, user.getNickname());
    sendMessageToUser(user, mess);
}



// COMMANDS

void    Server::message(const User &user, std::vector<std::string> const & args, std::string const &message) const
{
	if (args.size() < 1)
		return;
    std::string mess = CMD_PRIVMSG(user.getSender(), args[0], message);

    sendMessage(user, args[0], mess);
}


void	Server::who(User &user, std::vector<std::string> const &args) const
{
	std::string	mess;
	
	if (args.size() < 1)
		return ;
	std::string const &channelName = args[0];

	//check if channel exists
	Channel	*channel;
	try {
		channel = findChannelByName(channelName, user);
	
	} catch(const std::exception& e) {
		sendMessageToUser(user, e.what());
		return ;
	}
	
	channel->who(*this, user, true); // ???
}


void	Server::whoIs(User &requestingUser, std::vector<std::string> const &args) const
{
	std::string	mess;

	if (args.size() < 1)
		return ;
	std::string const & investigatedUserNick = args[0];

	User 	*investigatedUser;
	try
	{
		investigatedUser = findUserByNickname(investigatedUserNick, requestingUser);
	}
	catch(const std::exception& e)
	{
		sendMessageToUser(requestingUser, e.what());
		return ;
	}
	
	investigatedUser->whoIs(*this, requestingUser);
}


void	Server::notice(User &user, std::vector<std::string> const &	args)
{
	std::string	mess;
	User *userTarget;
	mess = CMD_NOTICE_TARGET(user.getSender(), mess, args[0]); // ???

	try
	{
		userTarget = findUserByNickname(args[0], user);
	}
	catch(const std::exception& e)
	{
		sendMessageToUser(user, e.what());
		return ;
	}
	sendMessageToUser(*userTarget, mess);
}



void	Server::userHost(User &user, std::vector<std::string> const &args)
{
	std::string		messtmp = "";

	std::string		mess;
	std::string 	infoTarget = "";
	if (args.size() == 0)
	{
		mess = RPL_USERHOST(user.getNickname(), infoTarget);
	}
	else
	{
		for (size_t i = 0; i < args.size(); i++)
		{
			User *userTarget;

			try
			{
				userTarget = findUserByNickname(args[i], user);
				std::string adress = inet_ntoa(userTarget->getAddr().sin_addr);
				infoTarget = args[i] + "=+~" + userTarget->getUsername() + "@" + adress;
				messtmp += infoTarget + " ";
			}
			catch(const std::exception& e){}
		}
		mess = RPL_USERHOST(user.getNickname(), messtmp);

	}

	sendMessageToUser(user, mess);
}



// CHANNELS COMMANDS

void	Server::joinChannel(User &user, std::vector<std::string> const &args)
{
	std::string channelName;
	std::string	password;
	std::string mess;
	Channel		*channel;
	bool 		op = false;
	std::string	chanBuffer;
	std::string	passBuffer;

	try {
		if (args.size() == 0 || args.size() > 2)
			throw needMoreParams(user.getNickname(), "JOIN");
	} catch(const std::exception& e) {
		sendMessageToUser(user, e.what());
		return ;
	}

	if (args.size() == 2)
		passBuffer = args[1];

	size_t	pos;
	chanBuffer = args[0];
	while (!chanBuffer.empty()) {
		pos = chanBuffer.find(',');
		if (pos ==  std::string::npos) {
			pos = chanBuffer.length();
			channelName = chanBuffer.substr(0, pos);
			chanBuffer.clear();

		} else {
			channelName = chanBuffer.substr(0, pos);
			chanBuffer = chanBuffer.substr(++pos, chanBuffer.length());
		}

		try {
			channel = findChannelByName(channelName, user);
		} catch(const std::exception& e) {
			channel = createChannel(user, channelName);
			op =  true;
		}

		if (args.size() == 2) {
			pos = passBuffer.find(',');
			if (pos ==  std::string::npos) {
				pos = passBuffer.length();
				password = passBuffer.substr(0, pos);
				passBuffer.clear();

			} else {
				password = passBuffer.substr(0, pos);
				passBuffer = passBuffer.substr(++pos, passBuffer.length());
			}
		}
		
		if (channel->addUser(*this, user, password, op))
			user.addChannel(channel);
	}
}


void	Server::partChannel(User &user,std::vector<std::string> const &args, std::string const &reason)
{
	Channel				*channel;
	std::string			mess;

	try {
		if (args.size() < 1)
			throw needMoreParams(user.getNickname(), "PART");
	
	} catch(const std::exception& e) {
		sendMessageToUser(user, e.what());
		return ;
	}

	std::string const	&channelName = args[0];
	try {
		channel = findChannelByName(channelName, user);
	
	} catch(const std::exception& e) {
		sendMessageToUser(user, e.what());
		return ;
	}

	if (channel->partUser(*this, user, reason)) {
		user.leaveChannel(channel);
	}
}


void	Server::inviteChannel(User &invitingUser, std::vector<std::string> const & args)
{
	std::string	mess;

	try 
	{
		if (args.size() < 2)
			throw needMoreParams(invitingUser.getNickname(), "INVITE");
	}
	catch(const std::exception& e)
	{
		sendMessageToUser(invitingUser, e.what());
		return ;
	}

	std::string const & invited = args[0];
	std::string const & channelName = args[1];
	
	Channel	*channel;
	try {
		channel = findChannelByName(channelName, invitingUser);
	} catch(const std::exception& e) {
		sendMessageToUser(invitingUser, e.what());
		return ;
	}

	User	*invitedUser;
	try
	{
		invitedUser = findUserByNickname(invited, invitingUser);
		channel->inviteUser(*this, *invitedUser, invitingUser);
		return ;
	}
	catch(const std::exception& e)
	{
		sendMessageToUser(invitingUser, e.what());
		return ;
	}
}


void	Server::kickChannel(User &kicker, std::vector<std::string> const & args, std::string const & reason) //peut etre merge avec part plus tard, mais la cest pour eviter les pb
{
	std::string	mess;

	try 
	{
		if (args.size() < 2)
			throw needMoreParams(kicker.getNickname(), "KICK");
	}
	catch(const std::exception& e)
	{
		sendMessageToUser(kicker, e.what());
		return ;
	}

	std::string const &channelName = args[0];
	std::string const &kickedNick = args[1];
	
	Channel	*channel;
	try {
		channel = findChannelByName(channelName, kicker);
	} catch(const std::exception& e) {
		sendMessageToUser(kicker, e.what());
		return ;
	}

	User 	*kickedUser;
	try
	{
		kickedUser = findUserByNickname(kickedNick, kicker);
		if (channel->kickUser(*this, *kickedUser, kicker, reason))
		{
			kickedUser->leaveChannel(channel);
			if (channel->isEmpty())
				removeChannel(channel);//inutil i think
		}
	}
	catch(const std::exception& e)
	{
		sendMessageToUser(kicker, e.what());
	}
	return ;
}


void	Server::topicChannel(User &user, std::vector<std::string> const & args, std::string const & topic)
{
	std::string	mess;
	
	try 
	{
		if (args.size() < 1)
			throw needMoreParams(user.getNickname(), "TOPIC");
	}
	catch(const std::exception& e)
	{
		sendMessageToUser(user, e.what());
		return ;
	}


	std::string const & channelName = args[0];

	Channel	*channel;

	try {
		channel = findChannelByName(channelName, user);
	} catch(const std::exception& e) {
		sendMessageToUser(user, e.what());
		return ;
	}

	channel->updateTopic(*this, user, topic);
}


void	Server::modeChannel(User &user, std::vector<std::string> const &args)
{
	std::string	mess;
	try 
	{
		if (args.empty())
			throw needMoreParams(user.getNickname(), "MODE");
	}
	catch(const std::exception& e)
	{
		sendMessageToUser(user, e.what());
		return ;
	}

	const std::string &target = args[0];
	if (target[0] != '#') {
		return ; // voir si on envoi un message
	}

	Channel	*channel;
	
	try {
		channel = findChannelByName(target, user);
	} catch(const std::exception& e) {
		sendMessageToUser(user, e.what());
		return ;
	}

	channel->updateMode(*this, user, args);
}
