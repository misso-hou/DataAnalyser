#include "animation.h"

#include <filesystem>
#include <iostream>

namespace modules {
namespace animation {

const float CMD_X_RANGE = 100;
const int DURATION = 50;
const int Y_RANGE = 10;

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
  time_array.push_back(duration_time*5);
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
  static vector<string> colors = {"r", "b","blueviolet","g" ,"cyan","gold"};
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

}
}