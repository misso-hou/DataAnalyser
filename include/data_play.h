#pragma once

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>


using namespace std;
using mesh2D = vector<vector<float>>;

//变量定义
int cycle_time_;
int start_index_;
int data_length_;
//提取数据容器
mesh2D data_mat_;
//键盘控制
bool pause_;
bool erase_;
bool back_;
bool blade_;

/*读csv数据函数*/
vector<vector<float>> RowDataReader(string file_name, int row_num, int bias_index);
// 按键控制相关函数
char GetKey();
string GetKeyWithTimeout(int timeout_ms);
