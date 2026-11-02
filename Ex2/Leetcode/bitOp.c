#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

void calculate_the_maximum(int n, int k)
{
    int m_and = 0;
    int m_or = 0;
    int m_xor = 0;

    for(int a = 1; a <= n; a++)
    {
        for(int b = a + 1; b <= n; b++)
        {
            int and = a & b;
            int or = a | b;
            int xor = a ^ b;

            if(and < k && and > m_and)
                m_and = and;

            if(or < k && or > m_or)
                m_or = or;

            if(xor < k && xor > m_xor)
                m_xor = xor;
        }
    }

    printf("%d\n", m_and);
    printf("%d\n", m_or);
    printf("%d\n", m_xor);
}

int main()
{
    int n, k;

    scanf("%d %d", &n, &k);

    calculate_the_maximum(n, k);

    return 0;
}
