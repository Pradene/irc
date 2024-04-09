#ifndef BOT_HPP
# define BOT_HPP

# include <string>
# include <iostream>
# include <fstream>
# include <deque>
# include <vector>

class Bot {

public:

	Bot(void);
	Bot(const char *path);
	~Bot(void);

	void	displayList() const;
	void	add(std::string word);
	void	remove(std::string word);

	std::string	filters(std::string message);
private:
	std::vector<std::string>	_list;
};

#endif