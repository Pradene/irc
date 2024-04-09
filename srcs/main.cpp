#include "Server.hpp"
#include "Utils.hpp"

bool	g_end;

void handleSignal(int signum) 
{
	(void) signum;
	g_end = true;
}

int main(int argc, char **argv)
{
	if (argc != 3)
		error("usage ./ircserv <port> <password>");
	signal(SIGINT, handleSignal);
	Server server(argv[1], argv[2], "badwords.txt");
	server.run();
	return (0);
}
