#pragma once

#include <plotIt.h>

class TCanvas;
class TObject;

namespace plotIt {
  class plotter {

    public:
      plotter(plotIt& plotIt):
        m_plotIt(plotIt) {

        }


      virtual bool plot(TCanvas& c, Plot& plot) = 0;
      virtual bool supports(TObject& object) = 0;

    protected:
      plotIt& m_plotIt;

  };
}
