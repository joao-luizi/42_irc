#ifndef DEBUG_HPP
# define DEBUG_HPP

# include <iostream>

# ifdef DEBUG
#  define DEBUG_MSG(x) std::cout << x << std::endl
# else
#  define DEBUG_MSG(x)
# endif

#endif