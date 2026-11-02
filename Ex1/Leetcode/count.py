#338. Counting Bits

class Solution:
    def countBits(self, n: int):
        a = []
        for x in range (n + 1):
            b = format(x, 'b')
            c = 0
            for x in b:
                if x == '1':
                    c=c+1
            a.append(c)
        return a

Example 1:

Input: n = 2
Output: [0,1,1]
Explanation:
0 --> 0
1 --> 1
2 --> 10
Example 2:

Input: n = 5
Output: [0,1,1,2,1,2]
Explanation:
0 --> 0
1 --> 1
2 --> 10
3 --> 11
4 --> 100
5 --> 101
