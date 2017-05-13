#include <iostream>
#include "vpstd.hpp"

bool is_prime (int x) {
    std::cout << "Calculating. Please, wait...\n";
    for (int i = 2; i < x; ++i) 
        if (x % i == 0) 
            return false;
    return true;
}

int main () {
    vpstd::future<bool> fut = vpstd::async(is_prime, 313222313);

    std::cout << "Checking whether 313222313 is prime.\n";

    bool ret = fut.get();

    if (ret) 
        std::cout << "It is prime!\n";
    else 
        std::cout << "It is not prime.\n";

    return 0;
}
