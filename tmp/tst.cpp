#include <iostream>
#include <cstdlib>
#include <string>

int main()
{
const char* env_p;
env_p = std::getenv("HOME");
if(nullptr == env_p) env_p = std::getenv("USERPROFILE");

std::string path = env_p;

   if(nullptr != env_p)
        std::cout << "Your PATH is: " << path << '\n';
   else 
     std::cout << "ooops! ";

  return 0;
}

//PathNameCfg += getenv("%%");