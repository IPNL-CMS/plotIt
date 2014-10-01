#pragma once

#include <plotter.h>

namespace plotIt {
  class TH1Plotter: public plotter {
    public:
      TH1Plotter(plotIt& plotIt):
        plotter(plotIt) {
        }

      virtual bool plot(TCanvas& c, Plot& plot);
      virtual bool supports(TObject& object);

    private:
      void setHistogramStyle(const File& file);
  };
}
