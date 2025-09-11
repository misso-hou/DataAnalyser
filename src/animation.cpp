#include "animation.h"

#include <filesystem>
#include <iostream>

#include "weighted_window_mode.h"

namespace modules {
namespace animation {

const float CMD_X_RANGE = 100;
const int DURATION = 50;
const int Y_RANGE = 10;
const float BAR_X = 3.0;

// canvas and flush events
auto canvas_update_flush_events = [](pybind11::object figure) {
  pybind11::object canvas_attr = figure.attr("canvas");
  pybind11::object canvas_update_attr = canvas_attr.attr("update");
  pybind11::object canvas_flush_events_attr = canvas_attr.attr("flush_events");
  pybind11::object ret01 = canvas_update_attr();
  pybind11::object ret02 = canvas_flush_events_attr();
};

// canvas_copy_from_bbox
auto canvas_copy_from_bbox = [](pybind11::object figure) -> pybind11::object {
  pybind11::object canvas_attr = figure.attr("canvas");
  pybind11::object canvas_copy_from_bbox_attr = canvas_attr.attr("copy_from_bbox");
  pybind11::object fig_bbox = figure.attr("bbox");
  pybind11::object ret = canvas_copy_from_bbox_attr(fig_bbox);
  return ret;
};

// canvas_restore_region
auto canvas_restore_region = [](pybind11::object figure, pybind11::object bg) {
  pybind11::object canvas_attr = figure.attr("canvas");
  pybind11::object canvas_restore_region_attr = canvas_attr.attr("restore_region");
  canvas_restore_region_attr(bg);
};

void Animation::InitializePlt() {
  /***速度监视器***/
  pybind11::dict cmd_kwargs("figsize"_a = py::make_tuple(14, 7), "dpi"_a = 100, "tight_layout"_a = true);
  CmdPltInit(cmd_kwargs, CMD_X_RANGE);
  BarPltInit(cmd_kwargs);
}

void Animation::BarPltInit(const pybind11::dict& fig_kwargs) {
  auto plt = mpl::pyplot::import();  
  auto [figure, axes] = plt.subplots();                              
  // mpl::figure::Figure figure = plt.figure(Args(), fig_kwargs); 
  bar_figure_ptr_ = make_shared<mpl::figure::Figure>(figure);    
  bar_axes_ptr_ = make_shared<mpl::axes::Axes>(axes);    
  bar_axes_ptr_->set_xlim(Args(-BAR_X/2, BAR_X/2));
  bar_axes_ptr_->set_ylim(Args(-100, 200.0));  
  plt.show(Args(), Kwargs("block"_a = 0));
  plt.grid(Args(true), Kwargs("linestyle"_a = "--", "linewidth"_a = 0.5, "color"_a = "black", "alpha"_a = 0.5));
  bar_axes_ptr_->set_title(Args("long window"));
  plt.pause(Args(0.1));
  bar_background_ = canvas_copy_from_bbox(bar_figure_ptr_->unwrap());
  //-----------
  // auto [figure02, axes02] = plt.subplots();                              
  // // mpl::figure::Figure figure = plt.figure(Args(), fig_kwargs); 
  // bar02_figure_ptr_ = make_shared<mpl::figure::Figure>(figure02);    
  // bar02_axes_ptr_ = make_shared<mpl::axes::Axes>(axes02);    
  // bar02_axes_ptr_->set_xlim(Args(-BAR_X/2, BAR_X/2));
  // bar02_axes_ptr_->set_ylim(Args(-0.1, 100.0));  
  // plt.show(Args(), Kwargs("block"_a = 0));
  // plt.grid(Args(true), Kwargs("linestyle"_a = "--", "linewidth"_a = 0.5, "color"_a = "black", "alpha"_a = 0.5));
  // bar02_axes_ptr_->set_title(Args("short window"));
  // plt.pause(Args(0.1));
  // bar02_background_ = canvas_copy_from_bbox(bar02_figure_ptr_->unwrap());
}

void Animation::CmdPltInit(const pybind11::dict& fig_kwargs, const float& x_axis_range) {
  data_plt_ = mpl::pyplot::import();                                
  mpl::figure::Figure figure = data_plt_.figure(Args(), fig_kwargs); 
  data_figure_ptr_ = make_shared<mpl::figure::Figure>(figure);
  auto data_gs = GridSpec(4, 1);  

  //axes01
  auto axes_obj_01 = figure.add_subplot(Args(data_gs(py::slice(0, 2, 1),0).unwrap()),Kwargs("facecolor"_a = "lightsalmon"));           
  data_axes01_ptr_ = make_shared<mpl::axes::Axes>(axes_obj_01);    
  data_axes01_ptr_->set_xlim(Args(-0.3f, x_axis_range));
  data_axes01_ptr_->set_ylim(Args(-1, 2.5));   
  data_plt_.show(Args(), Kwargs("block"_a = 0));
  data_plt_.grid(Args(true), Kwargs("linestyle"_a = "--", "linewidth"_a = 0.5, "color"_a = "black", "alpha"_a = 0.5));
  //axes02
  auto axes_obj_02 = figure.add_subplot(Args(data_gs(2,0).unwrap()));           
  data_axes02_ptr_ = make_shared<mpl::axes::Axes>(axes_obj_02);    
  data_axes02_ptr_->set_xlim(Args(-0.3f, x_axis_range));
  data_axes02_ptr_->set_ylim(Args(-1, 30));   
  data_plt_.show(Args(), Kwargs("block"_a = 0));
  data_plt_.grid(Args(true), Kwargs("linestyle"_a = "--", "linewidth"_a = 0.5, "color"_a = "black", "alpha"_a = 0.5));
  //axes03
  auto axes_obj_03 = figure.add_subplot(Args(data_gs(3,0).unwrap()));           
  data_axes03_ptr_ = make_shared<mpl::axes::Axes>(axes_obj_03);    
  data_axes03_ptr_->set_xlim(Args(-0.3f, x_axis_range));
  data_axes03_ptr_->set_ylim(Args(-0.5, 0.5));   
  data_plt_.show(Args(), Kwargs("block"_a = 0));
  data_plt_.grid(Args(true), Kwargs("linestyle"_a = "--", "linewidth"_a = 0.5, "color"_a = "black", "alpha"_a = 0.5));
  // data_axes01_ptr_->unwrap().attr("set_axis_off")();

  data_plt_.pause(Args(0.1));
  data_background_ = canvas_copy_from_bbox(data_figure_ptr_->unwrap());
}

void Animation::SetData(const vector<float>& new_data) {
  plt_data_ = new_data;
}

void Animation::Monitor(int buffer_length) {
  static bool once_flag = true;
  /******动画频率设置******/
  static float duration_time = 0.0f;
  static int64_t last_sim_time_stamp = 0;
  int64_t current_time_stamp = TimeToolKit::TimeSpecSysCurrentMs();
  // 时间控制
  int record_time = current_time_stamp - last_sim_time_stamp;
  if (record_time < DURATION && last_sim_time_stamp != 0) {
    return;
  }
  if (last_sim_time_stamp == 0) {
    duration_time = 0.0f;
  } else {
    duration_time += record_time / 1000.f;
  }
  last_sim_time_stamp = current_time_stamp;
  canvas_restore_region(data_figure_ptr_->unwrap(), data_background_);
  /******数据计算******/
  /*step01->实时数据更新*/
  static vector<float> time_array;
  static float test_tick = 0;
  test_tick += 0.4;
  time_array.push_back(test_tick);
  // time_array.push_back(duration_time*5);

  int data_num = plt_data_.size();
  static mesh2D line_data(data_num);
  static vector<py::object> lines_artist(data_num);
  for(uint i=0;i<data_num;i++){
    line_data[i].push_back(plt_data_[i]);
  }
  // 数据更新
  if (time_array.size() > buffer_length) {
    time_array.erase(time_array.begin());
    for(auto& line:line_data){
      line.erase(line.begin());
    }
  }
  /*step02->static artist生成*/
  static vector<string> colors = {"silver", "b","blueviolet","g" ,"cyan","gold","r"};
  if (once_flag) {
    once_flag = false;
    for (int i = 0; i < line_data.size(); i++) {
      if(i<2 || i>=4){
        lines_artist[i] = data_axes01_ptr_->plot(Args(time_array, line_data[i]), Kwargs("c"_a = colors[i], "lw"_a = 1.0)).unwrap().cast<py::list>()[0];
      }
      else if(i==2){
        lines_artist[i] = data_axes02_ptr_->plot(Args(time_array, line_data[i]), Kwargs("c"_a = colors[i], "lw"_a = 1.0)).unwrap().cast<py::list>()[0];
      }else if(i==3){
        lines_artist[i] = data_axes03_ptr_->plot(Args(time_array, line_data[i]), Kwargs("c"_a = colors[i], "lw"_a = 1.0)).unwrap().cast<py::list>()[0];
      }
    }
  }
  /*step03->artist实时数据更新并绘制*/
  for (int j = 0; j < lines_artist.size(); j++) {
    lines_artist[j].attr("set_data")(time_array, line_data[j]);
    if(j<2 || j>=4){
      data_axes01_ptr_->unwrap().attr("draw_artist")(lines_artist[j]);
    }
    else if(j==2){
      data_axes02_ptr_->unwrap().attr("draw_artist")(lines_artist[j]);
    }else if(j==3){
      data_axes02_ptr_->unwrap().attr("draw_artist")(lines_artist[j]);
    }
  }
  /******axis计算******/
  auto axes_xlim = data_axes01_ptr_->get_xlim();
  if (time_array.back() > get<1>(axes_xlim) - 10) {
    float x_min = get<1>(axes_xlim) - 20.f;
    float x_max = x_min + CMD_X_RANGE;
    data_axes01_ptr_->set_xlim(Args(x_min, x_max));
    data_axes02_ptr_->set_xlim(Args(x_min, x_max));
    data_axes03_ptr_->set_xlim(Args(x_min, x_max));
  }
  canvas_update_flush_events(data_figure_ptr_->unwrap());
}


void Animation::BarPlot01(const std::unordered_map<int, int>& frequency01,const std::unordered_map<int, int>& frequency02) {
  static bool once_flag = true;
  canvas_restore_region(bar_figure_ptr_->unwrap(), bar_background_);
  /******数据计算******/
  /*step02->static artist生成*/
  const int num = BAR_X/RESOLUTION;
  static vector<py::object> bar_artists(num);
  static vector<py::object> bar02_artists(num);
  // std::unordered_map<int, int> frequency;
  static vector<float> bin(num);
  static vector<float> nums(num);
  if(once_flag){
    once_flag = false;
    for(int i=0;i<num;i++){
      bin[i] = -BAR_X/2 + RESOLUTION*i;
      nums[i] = 0;
    }
    auto artists = bar_axes_ptr_->bar(Args(bin, nums, RESOLUTION/1.5),Kwargs("color"_a = "blue")).unwrap().cast<py::list>();
    auto artists02 = bar_axes_ptr_->bar(Args(bin, nums, RESOLUTION/1.5),Kwargs("color"_a = "green")).unwrap().cast<py::list>();
    for(int i=0;i<num;i++){
      bar_artists[i] = artists[i];
      bar02_artists[i] = artists02[i];
    }
  }
  /*step03->artist实时数据更新并绘制*/
  int max_num = 0;
  for (const auto pair : frequency01){
    float index = pair.first+BAR_X/(2*RESOLUTION);
    bar_artists[index].attr("set_height")(pair.second);
    bar_axes_ptr_->unwrap().attr("draw_artist")(bar_artists[index]);
    if(max_num<pair.second){
      max_num = pair.second;
    }
  }

  int max_num02 = 0;
  for (const auto pair : frequency02){
    float index = pair.first+BAR_X/(2*RESOLUTION);
    bar02_artists[index].attr("set_height")(-pair.second);
    bar_axes_ptr_->unwrap().attr("draw_artist")(bar02_artists[index]);
    if(max_num02<pair.second){
      max_num02 = pair.second;
    }
  }

  pybind11::list axes_ylim = bar_axes_ptr_->unwrap().attr("get_ylim")();
  int y_min = axes_ylim[0].cast<float>();
  int y_max = axes_ylim[1].cast<float>();
  bool change = false;
  if (max_num > (y_max - 5)) {
    y_max = max_num + 5;
    change = true;
  }
  if(-max_num02 < (y_min + 5)) {
    change = true;
    y_min = -max_num02 - 5;
  }
  if(change){
    bar_axes_ptr_->unwrap().attr("set_ylim")(Args(y_min, y_max));
  }

  canvas_update_flush_events(bar_figure_ptr_->unwrap());
}


// void Animation::BarPlot01(const std::unordered_map<int, int>& frequency) {
//   static bool once_flag = true;
//   canvas_restore_region(bar_figure_ptr_->unwrap(), bar_background_);
//   /******数据计算******/
//   /*step02->static artist生成*/
//   const int num = BAR_X/RESOLUTION;
//   static vector<py::object> bar_artists(num);
//   // std::unordered_map<int, int> frequency;
//   static vector<float> bin(num);
//   static vector<float> nums(num);
//   if(once_flag){
//     once_flag = false;
//     for(int i=0;i<num;i++){
//       bin[i] = -BAR_X/2 + RESOLUTION*i;
//       nums[i] = 0;
//     }
//     auto artists = bar_axes_ptr_->bar(Args(bin, nums, RESOLUTION/1.5)).unwrap().cast<py::list>();
//     for(int i=0;i<num;i++){
//       bar_artists[i] = artists[i];
//     }
//   }
//   /*step03->artist实时数据更新并绘制*/
//   int max_num = 0;
//   for (const auto pair : frequency){
//     float index = pair.first+BAR_X/(2*RESOLUTION);
//     bar_artists[index].attr("set_height")(pair.second);
//     bar_axes_ptr_->unwrap().attr("draw_artist")(bar_artists[index]);
//     if(max_num<pair.second){
//       max_num = pair.second;
//     }
//   }

//   pybind11::list axes_ylim = bar_axes_ptr_->unwrap().attr("get_ylim")();
//   if (max_num > axes_ylim[1].cast<float>() - 5) {
//     float y_min = 0;
//     float y_max = max_num + 5;
//     bar_axes_ptr_->unwrap().attr("set_ylim")(Args(0, y_max));
//   }

//   canvas_update_flush_events(bar_figure_ptr_->unwrap());
// }

void Animation::BarPlot02(const std::unordered_map<int, int>& frequency) {
  static bool once_flag = true;
  canvas_restore_region(bar02_figure_ptr_->unwrap(), bar02_background_);
  /******数据计算******/
  /*step02->static artist生成*/
  const int num = BAR_X/RESOLUTION;
  static vector<py::object> bar_artists(num);
  // std::unordered_map<int, int> frequency;
  static vector<float> bin(num);
  static vector<float> nums(num);
  if(once_flag){
    once_flag = false;
    for(int i=0;i<num;i++){
      bin[i] = -BAR_X/2 + RESOLUTION*i;
      nums[i] = 0;
    }
    auto artists = bar02_axes_ptr_->bar(Args(bin, nums, RESOLUTION/1.5)).unwrap().cast<py::list>();
    for(int i=0;i<num;i++){
      bar_artists[i] = artists[i];
    }
  }
  /*step03->artist实时数据更新并绘制*/
  int max_num = 0;
  for (const auto pair : frequency){
    float index = pair.first+BAR_X/(2*RESOLUTION);
    bar_artists[index].attr("set_height")(pair.second);
    bar02_axes_ptr_->unwrap().attr("draw_artist")(bar_artists[index]);
    if(max_num<pair.second){
      max_num = pair.second;
    }
  }

  pybind11::list axes_ylim = bar02_axes_ptr_->unwrap().attr("get_ylim")();
  if (max_num > axes_ylim[1].cast<float>() - 5) {
    float y_min = 0;
    float y_max = max_num + 5;
    bar02_axes_ptr_->unwrap().attr("set_ylim")(Args(0, y_max));
  }

  canvas_update_flush_events(bar02_figure_ptr_->unwrap());
}


}
}