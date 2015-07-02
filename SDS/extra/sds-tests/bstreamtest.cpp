#include "bstreamable.h"

#include <fstream>
#include <iostream>

int main()
{
    const unsigned long L = 666;
    

    std::ofstream of("tmp.bin",std::ios::binary);
    write(of,L);
    of.close();

    std::ifstream inf("tmp.bin",std::ios::binary);
    unsigned long l = L+1;
    read(inf,l);
    
    if (l!=L)
    {
        std::cout << "Test failed.";
    }
    else
    {   
        std::cout << "Test succeeded.";
    }

    return 0;
}
