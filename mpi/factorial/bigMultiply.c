#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <bigMultiply.h>

union u64
{
    uint64_t as64;
    struct
    {
        uint32_t lo;
        uint32_t hi;
    } as32;
};

void fdump(FILE *file, uint32_t *a, size_t len)
{
    fprintf(file, "len = %lu\n", len);
    fprintf(file, "0x");
    for (size_t i = 0; i < len; ++i)
    {
        fprintf(file, "%08x.", a[len - i - 1]);
    }
    fprintf(file, "\n");
}

uint32_t *bigMultiply(size_t *len_c, uint32_t *a, uint32_t *b, size_t len_a, size_t len_b)
{
    uint32_t *c = (uint32_t *)calloc(sizeof(uint32_t), (len_a + len_b));
    uint32_t *temp = (uint32_t *)calloc(sizeof(uint32_t), (len_a + 1));
    union u64 ans;
    uint32_t carry = 0;
    for (size_t j = 0; j < len_b; ++j)
    {
        carry = 0;
        for (size_t i = 0; i < len_a; ++i)
        {
            ans.as64 = (uint64_t)a[i] * (uint64_t)b[j] + carry;
            if (j == 0)
                c[i] = ans.as32.lo;
            else
                temp[i] = ans.as32.lo;
            carry = ans.as32.hi;
        }
        if (j == 0)
        {
            c[len_a] = carry;
        }
        else 
        {
            temp[len_a] = carry;
            carry = 0;
            for (size_t i = j; i - j < len_a + 1; ++i)
            {
                ans.as64 = (uint64_t)c[i] + (uint64_t)temp[i - j] + carry;
                c[i] = ans.as32.lo;
                carry = ans.as32.hi;
            }
            for (size_t i = len_a + j + 1; i < len_a + len_b; ++i)
            {
                ans.as64 = (uint64_t)c[i] + carry;
                c[i] = ans.as32.lo;
                carry = ans.as32.hi;
            }
        }
    }

    *len_c = len_a + len_b;
    for (size_t i = 0; i < len_a + len_b ; ++i)
    {
        if (c[len_a + len_b - i - 1])
        {
            *len_c = len_a + len_b - i;
            free(temp);
            return c;
        }
    }

    return NULL;
}

uint32_t *treeMultiply(size_t *ans_len, unsigned int start, unsigned int end)
{
    uint32_t *ans;
    if (start > end)
        return NULL;
    else if (start == end)
    {
        ans = (uint32_t *)malloc(sizeof(uint32_t));
        *ans = start;
        *ans_len = 1;
        return ans;
    }
    else if (end - start == 1)
    {
        union u64 temp_prod;
        temp_prod.as64 = (uint64_t)end * (uint64_t)start;
        if (temp_prod.as32.hi)
        {
            ans = (uint32_t *)malloc(2 * sizeof(uint32_t));
            *ans_len = 2;
            ans[1] = temp_prod.as32.hi;
        }
        else
        {
            ans = (uint32_t *)malloc(sizeof(uint32_t));
            *ans_len = 1;
        }
        ans[0] = temp_prod.as32.lo;
        return ans;
    }
    unsigned int middle = (start + end) / 2;
    size_t len_l, len_r;
    uint32_t *l = treeMultiply(&len_l, start, middle);
    uint32_t *r = treeMultiply(&len_r, middle + 1, end);
    ans = bigMultiply(ans_len, l, r, len_l, len_r);
    free(l);
    free(r);
    return ans;
}
