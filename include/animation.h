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
using matplotlibcpp17::gridspec::GridSpec;

using mesh2D = vector<vector<float>>;

class Animation : public utilities::Singleton<Animation> {
  friend class Singleton<Animation>;

 private:
  Animation()  {}
  ~Animation() {}

 public:
  void SetData(const vector<float>& new_data);
  void Monitor(int buffer_length);
  void InitializePlt();

 private:
  void CmdPltInit(const pybind11::dict& fig_kwargs, const float& x_axis_range);

 private:
  //画框
  mpl::pyplot::PyPlot data_plt_;
  //轴系
  shared_ptr<mpl::gridspec::GridSpec> data_gs_ptr_;
  shared_ptr<mpl::axes::Axes> data_axes01_ptr_;     //速度监视器轴系
  shared_ptr<mpl::axes::Axes> data_axes02_ptr_;     //速度监视器轴系
  shared_ptr<mpl::axes::Axes> data_axes03_ptr_;     //速度监视器轴系
  // figure
  shared_ptr<mpl::figure::Figure> data_figure_ptr_;
  // background
  py::object data_background_;
  py::object jet_cmap_;

  // data
  vector<float> plt_data_;

};
}  // namespace animation
}  // namespace modules
