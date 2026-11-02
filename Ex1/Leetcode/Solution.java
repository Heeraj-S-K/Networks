//421. Maximum XOR of Two Numbers in an Array

class Solution {
    public int findMaximumXOR(int[] a) {
        int m=0,k=0;
        for(int i=31;i>=0;i--){
            k|=1<<i;
            HashSet<Integer> s=new HashSet<>();
            for(int x:a) s.add(x&k);
            int c=m|(1<<i);
            for(int p:s){
                if(s.contains(p^c)){
                    m=c;
                    break;
                }
            }
        }
        return m;
    }
}

Example 1:

Input: nums = [3,10,5,25,2,8]
Output: 28
Explanation: The maximum result is 5 XOR 25 = 28.
Example 2:

Input: nums = [14,70,53,83,49,91,36,80,92,51,66,70]
Output: 127
