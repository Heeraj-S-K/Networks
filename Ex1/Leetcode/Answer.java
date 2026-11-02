//476. Number Complement

class Answer {
    public int findComplement(int num) {
        int m = 0;
        int t = num;
        while (t > 0) {
            m = (m << 1) | 1;
            t >>= 1;
        }
        return num ^ m;
    }
}

Example 1:

Input: num = 5
Output: 2
Explanation: The binary representation of 5 is 101 (no leading zero bits), and its complement is 010. So you need to output 2.
Example 2:

Input: num = 1
Output: 0
Explanation: The binary representation of 1 is 1 (no leading zero bits), and its complement is 0. So you need to output 0.
