/**
*   Big unsigned integer class with multiplication
*   
*   @file BigUInt.h
*
*   @date 02.2016
*   
*   @copyright GNU GPL v2.0
*
*   @author Viktor Prutyanov mailto:viktor.prutyanov@phystech.edu 
*/

#include <cstdio>
#include <vector>
#include <cstdint>
#include <string>

class BigUInt
{
    public:
        BigUInt(uint32_t startPart);
        ~BigUInt();
        uint32_t *Data()
        {
            return data.data();
        }
        size_t Length()
        {
            return data.size();
        }
        void Expand(uint32_t lastPart)
        {
            data.push_back(lastPart);
        }
        void Print(std::string tail);

    private:
        std::vector<uint32_t> data;
};

BigUInt::BigUInt(uint32_t firstPart)
    :data (std::vector<uint32_t>(1, firstPart))
{
    //nothing to do
}

BigUInt::~BigUInt()
{
    //nothing to do
}

BigUInt& operator*=(BigUInt &a, const uint32_t &b)
{
    size_t len = a.Length();
    uint64_t prod = 0;
    uint64_t carry = 0;

    for (size_t i = 0; i < len; ++i)
    {
        prod = (uint64_t)a.Data()[i] * (uint64_t)b + carry;
        a.Data()[i] = prod & 0x00000000FFFFFFFFllu;
        carry =      (prod & 0xFFFFFFFF00000000llu) >> 32;
    }

    if (carry > 0)
        a.Expand(carry);
    
    return a;
}

void BigUInt::Print(std::string tail)
{
    printf("0x");
    for (auto r = data.rbegin(); r < data.rend(); ++r)
    {
        printf("%08lx", *r);
    }
    printf(tail.c_str());
}
