#include "../lib/commandline.hpp"

int main(int argc, char** argv)
{
    if(argc < 1)
        return 0;
    CommandLine c(argv[1]);
    c.mainLoop();
}
