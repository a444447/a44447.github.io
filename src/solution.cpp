#include <vector>
#include <iostream>

class Solution
{
public:
    int removeElement(std::vector<int>& nums, int val) {
        int slow_ptr{0};
        for (int fast_ptr = 0; fast_ptr < nums.size(); ++fast_ptr)
        {
            if (nums[fast_ptr] != val)
            {
                nums[slow_ptr++] = nums[fast_ptr];
            }
        }
        return slow_ptr;
    }
};

int main()
{
    std::vector<int> a{0,1,2,2,3,0,4,2};
    Solution solution;
    std::cout << solution.removeElement(a, 2) << std::endl;
    return 0;
}