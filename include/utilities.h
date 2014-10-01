#pragma once

#include <defines.h>

#include <plotIt.h>

namespace plotIt {
  TStyle* createStyle();

  template<class T>
    void setAxisTitles(T* object, Plot& plot) {
      if (plot.x_axis.length() > 0 && object->GetXaxis()) {
        object->GetXaxis()->SetTitle(plot.x_axis.c_str());
      }

      if (plot.y_axis.length() > 0 && object->GetYaxis()) {
        float binSize = object->GetXaxis()->GetBinWidth(1);
        std::string title = plot.y_axis;
        std::stringstream ss;
        ss << title << " / " << std::fixed << std::setprecision(2) << binSize;
        object->GetYaxis()->SetTitle(ss.str().c_str());
      }

      if (plot.show_ratio && object->GetXaxis())
        object->GetXaxis()->SetLabelSize(0);
    }

  void setAxisTitles(TObject* object, Plot& plot);

  template<class T>
    void setDefaultStyle(T* object) {

      object->SetLabelFont(43, "XYZ");
      object->SetTitleFont(43, "XYZ");
      object->SetLabelSize(LABEL_FONTSIZE, "XYZ");
      object->SetTitleSize(TITLE_FONTSIZE, "XYZ");
      object->SetTickLength(0.03, "XYZ");

      object->GetYaxis()->SetTitle("Data / MC");
      object->GetYaxis()->SetNdivisions(510);
      object->GetYaxis()->SetTitleOffset(2.5);
      object->GetYaxis()->SetLabelOffset(0.01);
      object->GetYaxis()->SetTickLength(0.03);

      object->GetXaxis()->SetTitleOffset(3.5);
      object->GetXaxis()->SetLabelOffset(0.015);
      object->GetXaxis()->SetTickLength(0.03);
      
    }

  void setDefaultStyle(TObject* object);

  template<class T>
    float getMaximum(T* object) {
      return object->GetMaximum();
    }

  float getMaximum(TObject* object);

  template<class T>
    float getMinimum(T* object) {
      return object->GetMinimum();
    }

  float getMinimum(TObject* object);

  template<class T>
    void setMaximum(T* object, float minimum) {
      object->SetMaximum(minimum);
    }

  void setMaximum(TObject* object, float minimum);

  template<class T>
    void setMinimum(T* object, float minimum) {
      object->SetMinimum(minimum);
    }

  void setMinimum(TObject* object, float minimum);

  template<class T>
    void setRange(T* object, Plot& plot) {
      if (plot.x_axis_range.size() == 2)
        object->GetXaxis()->SetRangeUser(plot.x_axis_range[0], plot.x_axis_range[1]);
      if (plot.y_axis_range.size() == 2) {
        object->SetMinimum(plot.y_axis_range[0]);
        object->SetMaximum(plot.y_axis_range[1]);
      }
    }

  void setRange(TObject* object, Plot& plot);
}
