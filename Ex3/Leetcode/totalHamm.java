class Solution {
    public int totalHammingDistance(int[] nums) {
        int d = 0;
        int n = nums.length;
        for (int i = 0; i < 32; i++) {
            int c = 0;
            for (int num : nums) {
                c += (num >> i) & 1;
            }
            d += c * (n - c);
        }
        return d;
    }
}
