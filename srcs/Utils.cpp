#include "Utils.hpp"

/*
Displays error message on standard error output
*/
void	error(std::string err)
{
	if (!errno)
		errno = 1;
	std::cerr << "error: " << err << std::endl
	<< "error code: " << errno << std::endl;
	exit(errno);
}

/*
Set the given fd to non blocking, ie. 
the reading and writing operations 
associated with this fd will not block the program
even tho the data are not immediatly available
*/
int setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == 1)
		return (1);
	return (0);
}

/*
Print a vector of string into stdout
*/
void	displayStringVector(std::vector<std::string> strings)
{
	for (std::vector<std::string>::iterator it = strings.begin(); it != strings.end(); ++it)
        std::cout << *it << "   ";
    std::cout << std::endl;
}

int	toInt(std::string s) {
	std::stringstream	ss(s);
    int					value;

    ss >> value;
	if (ss.fail() || !ss.eof()) {
		return (-1);
	}

	return (value);
}