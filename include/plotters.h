#pragma once

#include <TH1Plotter.h>

namespace plotIt {
  static std::vector<std::shared_ptr<plotter>> s_plotters;
  void createPlotters(plotIt& plotIt) {
    s_plotters.push_back(std::make_shared<TH1Plotter>(plotIt));
  }

  bool plot(const File& file, TCanvas& c, Plot& plot) {
    for (auto& plotter: s_plotters) {
      if (plotter->supports(*file.object))
        return plotter->plot(c, plot);
    }

    return false;
  }
}
