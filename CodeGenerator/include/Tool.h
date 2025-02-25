//
// Created by 27306 on 24-10-4.
//

#ifndef TOOL_H
#define TOOL_H
#include <vector>
#include <algorithm>

class SortTool {
public:
    static void sortAscending(std::vector<int>& arr) {
        std::sort(arr.begin(), arr.end());
    }
};

#endif // TOOL_H

