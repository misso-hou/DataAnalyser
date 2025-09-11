#include "data_play.h"

#include <sys/select.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

#include "animation.h"
#include "facilities/base_time_struct.h"

#include <matplotlibcpp17/pyplot.h>
#include "weighted_window_mode.h"

#include <algorithm>
#include <vector>
#include <iomanip>

using namespace std;
using namespace matplotlibcpp17;

using namespace std;
namespace Anim = modules::animation;
Anim::Animation *Animator = Anim::Animation::GetInstance();

/**
 * @brief:按行读取csv数据(分组数据需要设置组中对应的行序列)
 * @param:
 *      file_name:csv文件路径
 *      row_num:csv每组数据的行数(例如三行为一组)
 *      bias_index:数据在csv数据组中对应的行索引
 * @return:csv数据转成的二维数组数据
 */
vector<vector<float>> RowDataReader(string file_name, int row_num, int bias_index) {
  ifstream file(file_name);  // csv文件导入
  if (!file.is_open()) {
    cout << file_name << "----->"
         << "文件不存在" << endl;
  }
  if (file.eof()) {
    cout << file_name << "----->"
         << "文件为空" << endl;
  }
  vector<vector<float>> all_data;
  string line;
  int line_index = 1;
  while (getline(file, line)) {  // 按行提取csv文件
    // 将障碍物数据中index的倍数行提取
    if ((line_index - bias_index) % row_num == 0) {  // note:障碍物三行数据为一组
      stringstream s1(line);
      string charItem;
      float fItem = 0.f;
      vector<float> one_line_data;
      /*提取行点集数据*/
      while (getline(s1, charItem, ',')) {
        fItem = stof(charItem);
        one_line_data.push_back(fItem);
      }
      all_data.push_back(one_line_data);
    } else {
      line_index++;
      continue;
    }
    line_index++;
  }
  // ifs.close();
  return all_data;
}

/**
 * @brief:获取一个字符输入(不需要按enter)
 * @return:键盘输入的字符
 */
char GetKey() {
  struct termios oldt, newt;
  char ch;
  tcgetattr(STDIN_FILENO, &oldt);           // 获取当前终端设置
  newt = oldt;                              // 复制当前设置
  newt.c_lflag &= ~(ICANON | ECHO);         // 禁用回显和规范模式
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // 设置新的终端设置
  ch = getchar();                           // 获取一个字符
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // 恢复原来的终端设置
  return ch;
}

/**
 * @brief:获取一个字符输入 (不需要按enter),并处理方向键的多字节序列
 * @return:键盘输入的字符
 */
string GetKeyWithTimeout(int timeout_ms) {
  struct termios oldt, newt;
  fd_set readfds;
  struct timeval tv;
  char ch;
  string input = "";

  // 获取当前终端设置
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);         // 禁用回显和规范模式
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // 设置新的终端设置

  FD_ZERO(&readfds);               // 清除文件描述符集
  FD_SET(STDIN_FILENO, &readfds);  // 将标准输入文件描述符添加到集

  // 设置超时时间(10毫秒)
  tv.tv_sec = 0;                   // 秒
  tv.tv_usec = timeout_ms * 1000;  // 微秒(10毫秒 = 10000微秒)

  // 使用 select() 检查是否有输入
  int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

  if (result > 0) {  // 如果有输入
    // 获取一个字符
    ch = getchar();
    input += ch;
    // 如果是ESC (27), 可能是方向键的开始字符
    if (ch == 27) {
      ch = getchar();  // 获取下一个字符
      input += ch;
      if (ch == '[') {
        ch = getchar();  // 获取方向键的最后一个字符
        input += ch;
      }
    }
  }
  // 恢复终端设置
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return input;
}

/**
 * @brief：数据播放键盘交互控制
 * @param:
 * @return: 键盘输入的字符
 */
bool KeyboardCtrl(int &index) {
  static int copy_time = 0;
  // 暂停&快进&后退控制
  back_ = false;
  if (!pause_) {
    string input = GetKeyWithTimeout(1);  //设置1秒超时
    // 暂停程序
    if (input == " ") {
      cout << "\033[33m 循环暂停，按任意键继续前进一帧，按<-后退一帧\33[0m" << endl;
      pause_ = true;
    }
    // 退出程序(按 'q' 退出循环)
    else if (input == "q") {
      cout << "退出程序!!!" << endl;
      return false;
    }
    // 上键速度X2
    else if (input == "\033[A") {
      cycle_time_ /= 2;
      cycle_time_ = max(1, cycle_time_);
      cout << "loop step cycle_time_ :" << cycle_time_ << "(us)" << endl;
    }
    // 下键速度/2
    else if (input == "\033[B") {
      cycle_time_ *= 2;
      cycle_time_ = min(cycle_time_, 100);
      cout << "loop step cycle_time_ :" << cycle_time_ << "(us)" << endl;
    }
    // 右键快进
    else if (input == "\033[C") {
      index += 300;
      index = min(data_length_ - 1, index);
      erase_ = true;
    }
    // 左键倒退
    else if (input == "\033[D") {
      index -= 100;
      index = max(index, 0);
      erase_ = true;
    }
    copy_time = cycle_time_;
  }
  // 空格暂停
  else {
    cycle_time_ = 5;  // 防止键盘输入等待
    //等待用户输入（持续等待）
    char key = GetKey();
    // 方向按键识别
    if (key == 27) {             // 方向键的起始字符是 ESC (27)
      char nextChar = GetKey();  // 获取方向键的第二个字符
      if (nextChar == '[') {
        nextChar = GetKey();  // 获取最后一个字符
        if (nextChar == 'D') {
          index--;
          index = max(index, 0);
          erase_ = true;
          back_ = true;
        }
      }
    }
    // 退出阻塞模式
    if (key == 'c') {
      cout << "继续循环..." << endl;
      pause_ = false;
      erase_ = false;
      cycle_time_ = copy_time;
    }
  }
  //数据结束后控制逻辑
  if (index == data_length_ - 1) {
    cout << "\033[31m !!!已显示全部数据!!! \33[0m" << endl;
    cout << "\033[32m 按 'c' 继续程序，'q' 退出程序。\33[0m" << endl;
    //等待用户输入
    char key = GetKey();
    if (key == 'c') {
      index = 0;
      erase_ = true;
    } else if (key == 'q') {
      cout << "!!!退出程序!!!" << endl;
      return false;  //按'q'退出循环
    } else {
      cout << "***再次确认下一步动作***" << endl;
      index--;
    }
  }

  return true;
}

void SetParam(int argc, char *argv[]) {
  //文件路径配置
  char currentPath[FILENAME_MAX];
  getcwd(currentPath, sizeof(currentPath));
  string current_path = string(currentPath);
  // 设置播放速度(默认周期20ms)
  cycle_time_ = argc >= 2 ? stoi(argv[1]) : 10;
  // 播放位置设置（tick累计)
  start_index_ = argc >= 3 ? atoi(argv[2]) : 0;
}

/**
 *@brief:csv数据提取
 */
void ExtractData() {
  //数据文件路径
  string data_file = "/home/workspace/csvLog/test_data.csv";
  //数据提取
  data_mat_ = RowDataReader(data_file, 1, 1);
  data_length_ = data_mat_.size();
}


float LowPassFilter01(const float& data,const float& alpha) {
  static bool first_flag = true;
  static float filtered_data = 0.0;

  if (first_flag) {  // first time enter
    first_flag = false;
    filtered_data = data;
  } else {
    filtered_data = alpha * data + (1.0f - alpha) * filtered_data;
  }
  return filtered_data;
}

float LowPassFilter02(const float& data,const float& alpha) {
  static bool first_flag = true;
  static float filtered_data = 0.0;

  if (first_flag) {  // first time enter
    first_flag = false;
    filtered_data = data;
  } else {
    filtered_data = alpha * data + (1.0f - alpha) * filtered_data;
  }
  return filtered_data;
}


/*
 * ---------数据回放使用方法----------：
 * 执行命令：./csvPlt+"播放速度设置“+”播放位置设置“+”csv文件夹序号“+”csv文件夹内部文件序号“
 * 播放速度默认为1
 * 播放位置默认从头开始
 * 文件夹默认为csv文件不带后缀序号
 * csv文件内部默认只有一组数据
 */
int main(int argc, char *argv[]) {
  WeightedWindows windows(800,200);
  pybind11::scoped_interpreter guard{};
  SetParam(argc, argv);
  ExtractData();
  Animator->InitializePlt();
  for (int i = start_index_; i < data_length_;)  //数据行遍历
  {
    //键盘控制
    if (!KeyboardCtrl(i)) break;
    int64_t start_time = TimeToolKit::TimeSpecSysCurrentMs();
    auto data_row = data_mat_[i];
    auto filter_torque01 = LowPassFilter01(data_row[1],0.05);
    auto filter_torque02 = LowPassFilter02(data_row[1],0.1);
    data_row.push_back(filter_torque01);
    data_row.push_back(filter_torque02);
    data_row[0]*=2;
    auto mode = windows.getWeightedMode(filter_torque02,data_row[2],data_row[0]);
    cout << "debug 02: tick->" << i << "; mode:" << mode << endl;
    data_row.push_back(mode);
    Animator->SetData(data_row);
    /*------动画显示-----*/
    Animator->Monitor(600);
    auto freq01 = windows.GetLongFreqency();
    // Animator->BarPlot01(freq01);
    auto freq02 = windows.GetShortFreqency();
    Animator->BarPlot01(freq01,freq02);
    int64_t end_time = TimeToolKit::TimeSpecSysCurrentMs();
    int64_t remaining_T = cycle_time_ - (end_time - start_time);
    if (remaining_T > 0) {
      this_thread::sleep_for(chrono::milliseconds(remaining_T));
    }
    if (!back_) i++;
    erase_ = false;
  }
  return 0;
  pybind11::finalize_interpreter();
}



// int main(int argc, char *argv[]) {
//   pybind11::scoped_interpreter guard{};
//   const vector<int> menMeans = {20, 35, 30, 35, -27};
//   const vector<int> womenMeans = {25, 32, 34, 20, -25};
//   const vector<int> menStd = {2, 3, 4, 1, 2};
//   const vector<int> womenStd = {3, 5, 2, 3, 3};
//   const vector<int> ind = {0, 1, 2, 3, 4}; // the x locations for the groups
//   const double width =
//       0.35; // the width of the bars: can also be len(x) sequence
//   auto plt = matplotlibcpp17::pyplot::import();
//   auto [fig, ax] = plt.subplots();
//   auto p1 = ax.bar(Args(ind, menMeans, width),
//                    Kwargs("yerr"_a = menStd, "label"_a = "Men"));
//   auto p2 = ax.bar(
//       Args(ind, womenMeans, width),
//       Kwargs("bottom"_a = menMeans, "yerr"_a = womenStd, "label"_a = "Women"));
//   ax.axhline(Args(0), Kwargs("color"_a = "grey", "linewidth"_a = 0.8));
//   ax.set_ylabel(Args("Scores"));
//   ax.set_title(Args("Scores by group and gender"));
//   ax.set_xticks(Args(ind, py::make_tuple("G1", "G2", "G3", "G4", "G5")));
//   ax.legend();

//   // // Label with label_type 'center' instead of the default 'edge'
//   // ax.bar_label(Args(p1.unwrap()), Kwargs("label_type"_a = "center"));
//   // ax.bar_label(Args(p2.unwrap()), Kwargs("label_type"_a = "center"));
//   // ax.bar_label(Args(p2.unwrap()));
//   plt.show();
//   pybind11::finalize_interpreter();
// }

