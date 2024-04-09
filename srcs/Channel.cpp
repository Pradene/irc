#include "Channel.hpp"



/******************************************************************************/
/*						CONSTRUCTORS & DESTRUCTORS							  */
/******************************************************************************/

/* Default constructor initializing channel's names and topic strings as empty*/
Channel::Channel() :
	_name(""),
	_userList(""),
	_userLimit(0),
	_inviteOnly(false),
	_creator(""),
	_creationTime(""),
	_topic(""),
	_topicUpdateTimestamp(""),
	_topicUpdateUser(""),
	_topicMode(false),
	_password(""),
	_passwordMode(false),
	_operatorList("")
{}

/* Parametrical constructor*/
Channel::Channel(const std::string name, const std::string creator, const std::string time) :
	_name(name),
	_userList(""),
	_userLimit(0),
	_inviteOnly(false),
	_creator(creator),
	_creationTime(time),
	_topic("Welcome to " + _name + "!"),
	_topicUpdateTimestamp(time),
	_topicUpdateUser(creator),
	_topicMode(false), // a check
	_password(""),
	_passwordMode(false),
	_operatorList("")
{}

/*Destructor*/
Channel::~Channel() {}

/******************************************************************************/
/*							SETTERS AND GETTER		 						  */
/******************************************************************************/

void	Channel::setName(std::string const & name) { _name = name; }

const std::string				&Channel::getName() const { return (_name); }
const std::map<User *, bool>	&Channel::getMembers() const { return (_members); }
bool							Channel::isEmpty() const { return (_members.empty()); }

/******************************************************************************/
/*							USERS MANAGEMENT								  */
/******************************************************************************/

void	Channel::removeUser(User &user) {
	std::map<User *, bool>::iterator it = _members.find(&user);
	if (it == _members.end())
		return ;
	_members.erase(it);
	updateUserList();
}

bool	Channel::addUser(Server const &server, User &user, std::string password, bool op)
{
	std::string mess;

	//il faut checker si je suis pas deja sur le channel, si cest le cas, return false
	if (userOnChannel(user) == true) {
		return (false);
	}

	if (_passwordMode && password != _password) {
		mess = ERR_CHANWRONGPASS(user.getNickname(), _name);
		server.sendMessageToUser(user, mess);
		return (false);
	}

	std::vector<User *>::iterator inviteIt = std::find(_pendingUserInvitations.begin(), _pendingUserInvitations.end(), &user);
	if (_inviteOnly && inviteIt == _pendingUserInvitations.end()) {
		mess = ERR_CHANNELUSERNOTINVIT(user.getNickname(), _name)
		server.sendMessageToUser(user, mess);
		// mess = CMD_NOTICE_TARGET(user.getNickname(), "This canal is on invite-only mode, you need to be invited to access it. If you think it is an error, you may contact one of the channel operator", _name);
		// server.sendMessageToUser(user, mess);
		return (false);

	} else if (_inviteOnly) {
		_pendingUserInvitations.erase(inviteIt);
	}
	
	if (_userLimit != 0 && _members.size() == _userLimit) {
		mess = ERR_CHANNELISFULL(user.getNickname(), _name);
		server.sendMessageToUser(user, mess);
		return (false);
	} else {
		_members.insert(std::make_pair(&user, op));
	}
	
	updateUserList();

	// notify every user about new joiner
	mess = CMD_JOIN(user.getSender(), _name);
	server.sendMessageToALL(user, _members, mess, false);

	mess = RPL_TOPIC(user.getNickname(), _name, _topic);
	server.sendMessageToUser(user, mess);

	mess = RPL_TOPICWHOTIME(user.getNickname(), _name, _topicUpdateUser, _topicUpdateTimestamp);
	server.sendMessageToUser(user, mess);

	mess = RPL_CREATIONTIME(user.getNickname(), _name, _creationTime);
	server.sendMessageToUser(user, mess);

	mess = RPL_NAMREPLY(user.getNickname(), _name, _userList);
	server.sendMessageToUser(user, mess);

	mess = RPL_ENDOFNAMES(user.getNickname(), _name);
	server.sendMessageToUser(user, mess);

	who(server, user, false);
	return (true);
}

bool	Channel::partUser(Server const &server, User &leavingUser, std::string const &reason) //add later raison
{
	std::string	mess;

	if (userOnChannel(leavingUser) == false) {
		mess = ERR_NOTONCHANNEL(leavingUser.getNickname(), _name);
		server.sendMessageToUser(leavingUser, mess);
		return (false);
	}

	mess = CMD_PART(leavingUser.getSender(), _name, reason);
	server.sendMessageToALL(leavingUser, _members, mess);
	
	std::map<User *, bool>::iterator it = _members.find(&leavingUser);
	_members.erase(it);
	updateUserList();

	return (true);
}

bool	Channel::kickUser(Server const &server, User &kickedUser, User &kickerUser, std::string const &reason)
{
	std::string	mess;
	std::string	name;
	
	if (userOnChannel(kickerUser) == false) {
		mess = ERR_NOTONCHANNEL(kickerUser.getNickname(), _name);
		server.sendMessageToUser(kickerUser, mess);
		return (false);
	}

	if (userIsOP(kickerUser) == false) {
		mess = ERR_CHANOPRIVSNEEDED(kickerUser.getNickname(), _name);
		server.sendMessageToUser(kickerUser, mess);
		return (false);
	}

	if (userOnChannel(kickedUser) == false) {
		return (false);
	}

	name = kickedUser.getNickname();
	mess = CMD_KICK(kickerUser.getSender(), _name, name, reason);
	server.sendMessageToALL(kickerUser, _members, mess);

	std::map<User *, bool>::iterator it = _members.find(&kickedUser);
	_members.erase(it);
	updateUserList();

	return (true);
}

void	Channel::inviteUser(Server const &server, User &invitedUser, User &invitingUser)
{
	std::string	mess;

	//check si inviting user est sur le channel
	if (userOnChannel(invitingUser) == false)
	{
		mess = ERR_NOTONCHANNEL(invitingUser.getNickname(), _name);
		server.sendMessageToUser(invitingUser, mess);
		return ;
	}

	//checking if invitingUser is operator
	if (_inviteOnly && userIsOP(invitingUser) == false)
	{
		mess = ERR_CHANOPRIVSNEEDED(invitingUser.getNickname(), _name);
		server.sendMessageToUser(invitingUser, mess);
		return ;
	}

	_pendingUserInvitations.push_back(&invitedUser);

	//sending invitation to the invited user
	mess = CMD_INVITE(invitingUser.getSender(), invitedUser.getNickname(), _name);
	server.sendMessageToUser(invitedUser, mess);

	//sending success notification to the inviting user
	mess = RPL_INVITE(invitingUser.getSender(), invitedUser.getNickname(), _name);
	server.sendMessageToUser(invitingUser, mess);

	//sending a notice to inform canal users about the invitation
	std::string target = "@" + _name;
	std::string message = invitingUser.getNickname() + " invited " + invitedUser.getNickname() + " into channel " + _name;
	mess = CMD_NOTICE_TARGET(SERVER_NAME, message , target)
	server.sendMessageToALL(invitingUser, _members, mess);
}

void    Channel::updateTopic(Server const &server, User &user, std::string const &topic)
{
	std::string	mess;

	//check si user est dans le canal
	if (userOnChannel(user) == false)
	{
		mess = ERR_NOTONCHANNEL(user.getNickname(), _name);
		server.sendMessageToUser(user, mess);
		return ;
	}

	if (_topicMode)
	{
		//check si user est operateur dans le channel
		if (userIsOP(user) == false)
		{
			mess = ERR_CHANOPRIVSNEEDED(user.getNickname(), _name);
			server.sendMessageToUser(user, mess);
			return ;
		}
	}

	//update topic
	_topic = topic;

	//update topic time
    std::time_t timestamp = std::time(NULL);
	std::stringstream creationTime;
	creationTime << timestamp;
	_topicUpdateTimestamp =  creationTime.str();

	//update topic changer
	_topicUpdateUser =  user.getNickname();

	//send message to every user of the channel to notify about the changes
	mess = CMD_TOPIC(user.getSender(), _name, _topic);
	server.sendMessageToALL(user, _members, mess);
}

/* Mode possible option :
- i : set/remove invite only
- t : set/remove restriction on topic modification
- k : set/remove channel password
- o : give or take channel operator privilege
- l : set/remove user limit
*/
void	Channel::updateMode(Server const &server, User &user, std::vector<std::string> const &args) {

	std::string	options;
	int			change = -1;
	size_t		it = 2;
	std::string	mess;

	if (args.size() >= 2)
		options = args[1];
	else
		return ;

	if (userIsOP(user) == false) {
		mess = ERR_CHANOPRIVSNEEDED(user.getNickname(), _name);
		server.sendMessageToUser(user, mess);
		return ;
	}


	for (size_t i = 0; options[i]; ++i) {
		if (options[i] == '+') {
			change = 1;
			++i;
		} else if (options[i] == '-') {
			change = 0;
			++i;
		}

		if (change == -1) {
			return ;
		}

		if (options[i] == 'i') {
			_inviteOnly = change;
			mess = CMD_MODE(user.getSender(), _name, ((change) ? "+" : "-"), options[i], "");
			server.sendMessageToALL(user, _members, mess);

		} else if (options[i] == 't') {
			_topicMode = change;
			mess = CMD_MODE(user.getSender(), _name, ((change) ? "+" : "-"), options[i], "");
			server.sendMessageToALL(user, _members, mess);

		} else if (options[i] == 'k') {
			if (change == 1 && args.size() < it + 1)
				return ;
			_passwordMode = change;

			mess = CMD_MODE(user.getSender(), _name, ((change) ? "+" : "-"), options[i], ((change) ? args[it] : ""));
			server.sendMessageToALL(user, _members, mess);
			if (change == 0) {
				_password = "";
				continue ;
			} else {
				_password = args[it++];
			}
		
		} else if (options[i] == 'o') {
			if (args.size() < it + 1)
				return ;

			try {
				User	*target = server.findUserByNickname(args[it++], user);
				if (userOnChannel(*target) == false) {
					mess = ERR_CHANNOTINLIST(user.getSender(), _name, target->getNickname());
					server.sendMessageToUser(user, mess);
					return ;
				}

				std::map<User *, bool>::iterator targetIt = _members.find(target);
				targetIt->second = change;
				updateUserList();

				mess = CMD_MODE(user.getSender(), _name, ((change) ? "+" : "-"), options[i], target->getNickname());
				server.sendMessageToALL(user, _members, mess);
			
			} catch(const std::exception& e) {
				server.sendMessageToUser(user, e.what());
				return ;
			}

		} else if (options[i] == 'l') {
			if (args.size() < it + 1)
				return ;

			int	limit = toInt(args[it++]);
			if (limit < 0)
				return ;

			_userLimit = limit;
			mess = CMD_MODE(user.getSender(), _name, ((change) ? "+" : "-"), options[i], ((change) ? args[it] : ""));
			server.sendMessageToALL(user, _members, mess);
		}
	}
	return ;
}

/* Updates user list of the canal, adds @ in front of operator users */
void	Channel::updateUserList()
{
	_userList.clear();
	for (std::map<User *, bool>::iterator it = _members.begin(); it != _members.end(); it++)
	{
		User *user = it->first;
		bool op = it->second;
		if (op)
			_userList.append("@");
		_userList.append(user->getNickname() + " ");
	}
	return ;
}

/******************************************************************************/
/*							INFORMATION ABOUT USERS							  */
/******************************************************************************/

// checking if user is on channel
bool	Channel::userOnChannel(User &user) {
	std::map<User *, bool>::iterator	it = _members.find(&user);

	if (it == _members.end()) {
		return (false);
	}

	return (true);
}

// checking if user is operator
bool	Channel::userIsOP(User &user) {
	std::map<User *, bool>::iterator	it = _members.find(&user);

	if (it != _members.end() && it->second == false) {
		return (false);
	}

	return (true);
}

void    Channel::who(Server const & server, User &user, bool checkNeeded)
{
	std::string mess;

	//check si user est sur le canal
	if (checkNeeded && userOnChannel(user) == false)
	{
		mess = ERR_NOTONCHANNEL(user.getNickname(), _name);
		server.sendMessageToUser(user, mess);
		return ;
	}

	//send one message per information to the user requesting information about channel users
	std::string	informationUserList;
	for (std::map<User *, bool>::iterator it = _members.begin(); it != _members.end(); it++)
	{
		User	&userChan = *(it->first);
		bool	isChanOp = it->second;
		informationUserList = userChan.getNickname() + " " + userChan.getSender() + " " + userChan.getUsername() + " :" + (isChanOp ? "@" : "");
		mess = RPL_WHOREPLY(user.getNickname(), _name, informationUserList);
		server.sendMessageToUser(user, mess);
	}
	mess = RPL_ENDOFWHO(user.getNickname(), _name);
	server.sendMessageToUser(user, mess);
}
