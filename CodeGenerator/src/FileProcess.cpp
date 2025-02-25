//
// Created by PengXinyang on 24-9-22.
//

#include "../include/FileProcess.h"
#include<sstream>
#include<fstream>
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <vector>
//从文件读入到string里


// 从指定文件读取内容并返回为字符串
string FileProcess::readFileContent(const char* filePath) {
    std::ifstream inputFile(filePath); // 创建输入文件流
    std::ostringstream stringBuffer;    // 用于存储文件内容的字符串流
    char currentChar;
    int temp;

    //输出当前的工作路径

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    } else {
        perror("getcwd() error");
    }

    // 检查文件是否成功打开
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";  // 返回空字符串表示文件未能成功打开
    }
    // 循环读取文件中的每个字符，直到文件结束
    while (inputFile.get(currentChar)) {
        stringBuffer << currentChar; // 将字符写入字符串流
    }
   // printf("file content in process is \n %s",stringBuffer.str().c_str());
    // 返回字符串流中的内容
    return stringBuffer.str();

}