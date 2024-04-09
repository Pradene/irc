#include "Server.hpp"
#include "User.hpp"
#include "Utils.hpp"

/******************************************************************************/
/*							REPLIES MANAGEMENT							  */
/******************************************************************************/

/*
Sends a formatted welcome message to
a newly connected user to the server.
*/
void    Server::welcome(User &user) const
{
	std::string message = _BOLD _MAGENTA"Welcome to our IRC server!\n";
	message += _MAGENTA"⠀⠀⠀⠀⣀⡤⢤⣄⠀⣠⡤⣤⡀⠀⠀⠀\n";
	message += _MAGENTA"⠀⠀⢀⣴⢫⠞⠛⠾⠺⠟⠛⢦⢻⣆⠀⠀\n";
	message += _MAGENTA"⠀⠀⣼⢇⣻⡀⠀⠀⠀⠀⠀⢸⡇⢿⣆⠀\n";
	message += _MAGENTA"⠀⢸⣯⢦⣽⣷⣄⡀⠀⢀⣴⣿⣳⣬⣿⠀\n";
	message += _MAGENTA"⢠⡞⢩⣿⠋⠙⠳⣽⢾⣯⠛⠙⢹⣯⠘⣷\n";
	message += _MAGENTA"⠀⠈⠛⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠋⠁\n";
	message += _ITALIC _MAGENTA"                    Created by @mudoh, @lpradene and @madavid\n" _RESET;

	std::string mess;
	mess = RPL_WELCOME(user.getNickname(), message);
	sendMessageToUser(user, mess);
		// error("Failed to send message to client");
	
	mess = RPL_YOURID(user.getNickname());
	sendMessageToUser(user, mess);

	mess = RPL_YOURHOST(user.getNickname());
	sendMessageToUser(user, mess);

	mess = RPL_MYINFO(user.getNickname());
	sendMessageToUser(user, mess);

	//005
}
