#include <cstdio>
#include <cerrno>
#include <string>
#include <gmpxx.h> 
#include <iostream>
#include <vector>

mpz_class treeFactorial(unsigned int start, unsigned int end)
{
    if (start > end)
        return 1;
    else if (start == end)
        return start;
    else if (start - end == 1)
        return (mpz_class)end * start;
    int middle = (start + end) / 2;
    return treeFactorial(start, middle) * treeFactorial(middle + 1, end);
}

unsigned long int get_ulong_opt(char *optarg)
{
    char *endptr;
    errno = 0;
                
    unsigned long int arg = strtoul(optarg, &endptr, 10);

    if (*endptr != '\0')
        errno = EINVAL;
   
    return arg;
}

int main(int argc, char *argv[])
{
    unsigned int n = get_ulong_opt(argv[1]);

    mpz_class a = treeFactorial(2, n);
    
    FILE *out_file = fopen("answer1.txt", "wb");
    gmp_fprintf(out_file, "%Zx\n", a.get_mpz_t(), 8);
    fclose(out_file);

    return 0;
}
