class Solution:
    def addBinary(self, a: str, b: str):
        d = int(a, 2) + int(b, 2)
        return format(d, 'b')
