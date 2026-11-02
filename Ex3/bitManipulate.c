int findLonely(vector<int> arr) {
    int unique_element = 0;
    
    for (int num : arr) {
        unique_element ^= num;
    }
    return unique_element;
}
