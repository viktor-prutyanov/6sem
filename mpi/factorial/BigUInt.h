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
        BigUInt(uint32_t startBlock);
        ~BigUInt();
        const uint32_t *Data() const
        {
            return data.data();
        }
        size_t Length() const
        {
            return data.size();
        }
        void Expand(uint32_t lastBlock)
        {
            data.push_back(lastBlock);
        }
        uint32_t GetBlock(size_t offset) const
        {
            if (offset < data.size())
                return data[offset];
            else
                return 0;
        }
        void SetOrExpandBlock(size_t offset, uint32_t value) //only for step-by-step access
        {
            if (offset < data.size())
                data[offset] = value;
            else
                Expand(value);
        }
        void ShiftBlocksLeft(size_t shift)
        {
            for (size_t i = 0; i < shift; ++i)
                data.emplace(data.begin());
        }
        void Print(std::string tail) const;
        static BigUInt MultiplyByUInt32(const BigUInt &a, uint32_t b);

    private:
        std::vector<uint32_t> data;
};

BigUInt::BigUInt(uint32_t firstBlock)
    :data (std::vector<uint32_t>(1, firstBlock))
{
    //nothing to do
}

BigUInt::~BigUInt()
{
    //nothing to do
}

BigUInt operator+(const BigUInt &a, const BigUInt &b)
{
    BigUInt c(0);

    size_t len = std::max(a.Length(), b.Length());
    uint64_t sum = 0;
    uint64_t carry = 0;

    for (size_t i = 0; i < len; ++i)
    {
        sum = (uint64_t)a.GetBlock(i) + (uint64_t)b.GetBlock(i) + carry;
        c.SetOrExpandBlock(i, sum & 0x00000000FFFFFFFFllu);
        carry = (sum & 0xFFFFFFFF00000000llu) >> 32;
    }
    
    if (carry > 0)
        c.Expand(carry);

    return c;
}

BigUInt& operator+=(BigUInt &a, BigUInt &b)
{
    size_t len = std::max(a.Length(), b.Length());
    uint64_t sum = 0;
    uint64_t carry = 0;

    for (size_t i = 0; i < len; ++i)
    {
        sum = (uint64_t)a.GetBlock(i) + (uint64_t)b.GetBlock(i) + carry;
        a.SetOrExpandBlock(i, sum & 0x00000000FFFFFFFFllu);
        carry = (sum & 0xFFFFFFFF00000000llu) >> 32;
    }
    
    if (carry > 0)
        a.Expand(carry);

    return a;
}

static BigUInt MultiplyByUInt32(const BigUInt &a, uint32_t b)
{
    BigUInt c(0);

    size_t len = a.Length();
    uint64_t prod = 0;
    uint64_t carry = 0;

    for (size_t i = 0; i < len; ++i)
    {
        prod = (uint64_t)a.GetBlock(i) * (uint64_t)b + carry;
        c.SetOrExpandBlock(i, prod & 0x00000000FFFFFFFFllu);
        carry = (prod & 0xFFFFFFFF00000000llu) >> 32;
    }

    if (carry > 0)
        c.Expand(carry);
    
    return c;
}

BigUInt operator*(const BigUInt &a, uint32_t b)
{
    return MultiplyByUInt32(a, b);
}

BigUInt operator*(const BigUInt &a, const BigUInt &b)
{
    BigUInt c(0);
    size_t len = a.Length();
   
    for (size_t i = 0; i < len; ++i)
    {
        BigUInt prod = MultiplyByUInt32(b, a.GetBlock(i));
        prod.ShiftBlocksLeft(i);
        c += prod;
    }

    return c;
}

BigUInt& operator*=(BigUInt &a, uint32_t &b)
{
    size_t len = a.Length();
    uint64_t prod = 0;
    uint64_t carry = 0;

    for (size_t i = 0; i < len; ++i)
    {
        prod = (uint64_t)a.GetBlock(i) * (uint64_t)b + carry;
        a.SetOrExpandBlock(i, prod & 0x00000000FFFFFFFFllu);
        carry = (prod & 0xFFFFFFFF00000000llu) >> 32;
    }

    if (carry > 0)
        a.Expand(carry);
    
    return a;
}

void BigUInt::Print(std::string tail) const
{
    printf("0x");
    for (auto r = data.rbegin(); r < data.rend(); ++r)
    {
        printf("%08x", *r);
    }
    printf(tail.c_str());
}
