#pragma once

#include <iostream>
#include <matplotlibcpp17/axes.h>
#include <matplotlibcpp17/figure.h>
#include <matplotlibcpp17/pyplot.h>
#include <memory>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <vector>


#include "facilities/base_time_struct.h"
#include "facilities/singleton.h"

namespace modules {
namespace animation {

using namespace std;
namespace py = pybind11;
namespace mpl = matplotlibcpp17;

using mesh2D = vector<vector<float>>;

class Animation : public utilities::Singleton<Animation> {
  friend class Singleton<Animation>;

 private:
  Animation()  {}
  ~Animation() {}

 public:
  void SetData(const float& speed, const float& yaw_rate, const float& torque, const float& angle);
  void Monitor(int buffer_length);
  void InitializePlt();

 private:
  void CmdPltInit(const pybind11::dict& fig_kwargs, const float& x_axis_range);

 private:
  //画框
  mpl::pyplot::PyPlot cmd_plt_;
  //轴系
  shared_ptr<mpl::axes::Axes> cmd_axes_ptr_;     //速度监视器轴系
  // figure
  shared_ptr<mpl::figure::Figure> cmd_figure_ptr_;
  // background
  py::object cmd_background_;
  py::object jet_cmap_;

  // data
  float speed_; 
  float yaw_rate_; 
  float torque_; 
  float angle_;

};
}  // namespace animation
}  // namespace modules
