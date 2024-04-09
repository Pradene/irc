#ifndef _UTILS_HPP
# define _UTILS_HPP

# include <iostream>
# include <sstream>

# include <stdlib.h>

# include <cerrno>

# include <string>

# include <vector>

# include <unistd.h>
# include <fcntl.h>

# include <sys/socket.h>

void	error(std::string err);
int 	setNonBlocking(int fd);
void	displayStringVector(std::vector<std::string> strings);
int	    toInt(std::string s);

#endif