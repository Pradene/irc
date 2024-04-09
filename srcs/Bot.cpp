#include "../includes/Bot.hpp"

/******************************************************************************/
/*						CONSTRUCTORS & DESTRUCTORS							  */
/******************************************************************************/

Bot::Bot(void)
{
}

Bot::Bot(const char *path)
{
	std::ifstream source(path);
	std::string buffer;
	while (getline(source, buffer))
	{
		_list.push_back(buffer);
		buffer.erase();
	}
}

Bot::~Bot(void)
{
}

/******************************************************************************/
/*								PUBLIC										  */
/******************************************************************************/

void Bot::displayList() const
{
	size_t i = 0;
	while (i < _list.size())
	{
		std::cout << _list[i] << std::endl;
		i++;
	}
}

void	Bot::add(std::string word)
{
	size_t i = 0;
	while (i < _list.size())
	{
		if (_list[i] == word)
			return ;
		i++;
	}
	_list.push_back(word);
}

void	Bot::remove(std::string word)
{
    size_t i = 0;
    while (i < _list.size())
    {
        if (_list[i] == word)
        {
            std::vector<std::string>::iterator it = _list.begin();
            std::advance(it, i);
            _list.erase(it);
            return;
        }
        i++;
    }
    _list.push_back(word);
}

std::string	Bot::filters(std::string message)
{
    std::string filteredMessage = message;
	size_t pos;
	std::string word;
	for (size_t i = 0; i < _list.size(); i++)
	{
		word = _list[i] + " ";
		pos = filteredMessage.find(word);
		if (pos != std::string::npos)
		{
			filteredMessage.replace(pos, word.size(), "BIIIIP ");
		}
	}
    return (filteredMessage);
}
