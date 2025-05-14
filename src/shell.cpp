#include <windows.h>
#include<iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <conio.h>
using namespace std;

void shellLoop(){
    string input;
    cout<< "Welcome to MyShell! Type 'myexit' to quit.\n";
            while (true) {
            char ch = _getch();

            if (ch == 13) { 
                std::cout << std::endl;
                break;
            } else if (ch == 8) { 
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b";
                }
            } else {
                input += ch;
                std::cout << ch;
            }
        }
}