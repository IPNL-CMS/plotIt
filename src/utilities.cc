#include <utilities.h>

#include <TH1.h>
#include <THStack.h>
#include <TStyle.h>

namespace plotIt {

  TStyle* createStyle() {
    TStyle *style = new TStyle("style", "style");

    // For the canvas:
    style->SetCanvasBorderMode(0);
    style->SetCanvasColor(kWhite);
    style->SetCanvasDefH(800); //Height of canvas
    style->SetCanvasDefW(800); //Width of canvas
    style->SetCanvasDefX(0);   //POsition on screen
    style->SetCanvasDefY(0);

    // For the Pad:
    style->SetPadBorderMode(0);
    style->SetPadColor(kWhite);
    style->SetPadGridX(false);
    style->SetPadGridY(false);
    style->SetGridColor(0);
    style->SetGridStyle(3);
    style->SetGridWidth(1);

    // For the frame:
    style->SetFrameBorderMode(0);
    style->SetFrameBorderSize(1);
    style->SetFrameFillColor(0);
    style->SetFrameFillStyle(0);
    style->SetFrameLineColor(1);
    style->SetFrameLineStyle(1);
    style->SetFrameLineWidth(1);

    // For the histo:
    style->SetHistLineColor(1);
    style->SetHistLineStyle(0);
    style->SetHistLineWidth(1);

    style->SetEndErrorSize(2);
    //  style->SetErrorMarker(20);
    //style->SetErrorX(0);

    style->SetMarkerStyle(20);

    //For the fit/function:
    style->SetOptFit(1);
    style->SetFitFormat("5.4g");
    style->SetFuncColor(2);
    style->SetFuncStyle(1);
    style->SetFuncWidth(1);

    //For the date:
    style->SetOptDate(0);

    // For the statistics box:
    style->SetOptFile(0);
    style->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
    style->SetStatColor(kWhite);
    style->SetStatFont(43);
    style->SetStatFontSize(0.025);
    style->SetStatTextColor(1);
    style->SetStatFormat("6.4g");
    style->SetStatBorderSize(1);
    style->SetStatH(0.1);
    style->SetStatW(0.15);

    // Margins:
    style->SetPadTopMargin(TOP_MARGIN);
    style->SetPadBottomMargin(BOTTOM_MARGIN);
    style->SetPadLeftMargin(LEFT_MARGIN);
    style->SetPadRightMargin(RIGHT_MARGIN);

    // For the Global title:
    style->SetOptTitle(0);
    style->SetTitleFont(63);
    style->SetTitleColor(1);
    style->SetTitleTextColor(1);
    style->SetTitleFillColor(10);
    style->SetTitleFontSize(TITLE_FONTSIZE);

    // For the axis titles:

    style->SetTitleColor(1, "XYZ");
    style->SetTitleFont(43, "XYZ");
    style->SetTitleSize(TITLE_FONTSIZE, "XYZ");
    style->SetTitleXOffset(3.5);
    style->SetTitleYOffset(2.5);

    style->SetLabelColor(1, "XYZ");
    style->SetLabelFont(43, "XYZ");
    style->SetLabelOffset(0.01, "YZ");
    style->SetLabelOffset(0.015, "X");
    style->SetLabelSize(LABEL_FONTSIZE, "XYZ");

    style->SetAxisColor(1, "XYZ");
    style->SetStripDecimals(kTRUE);
    style->SetTickLength(0.03, "XYZ");
    style->SetNdivisions(510, "XYZ");
    style->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
    style->SetPadTickY(1);

    style->SetOptLogx(0);
    style->SetOptLogy(0);
    style->SetOptLogz(0);

    style->SetHatchesSpacing(1.3);
    style->SetHatchesLineWidth(1);

    style->cd();

    return style;
  }

  void setAxisTitles(TObject* object, Plot& plot) {
    if (dynamic_cast<TH1*>(object))
      setAxisTitles(dynamic_cast<TH1*>(object), plot);
    else if (dynamic_cast<THStack*>(object))
      setAxisTitles(dynamic_cast<THStack*>(object), plot);
  }

  void setDefaultStyle(TObject* object, float topBottomScaleFactor) {
    if (dynamic_cast<TH1*>(object))
      setDefaultStyle(dynamic_cast<TH1*>(object), topBottomScaleFactor);
    else if (dynamic_cast<THStack*>(object))
      setDefaultStyle(dynamic_cast<THStack*>(object)->GetHistogram(), topBottomScaleFactor);
  }

  void hideXTitle(TObject* object) {
    if (dynamic_cast<TH1*>(object))
      hideXTitle(dynamic_cast<TH1*>(object));
    else if (dynamic_cast<THStack*>(object))
      hideXTitle(dynamic_cast<THStack*>(object));
  }

  float getMaximum(TObject* object) {
    if (dynamic_cast<TH1*>(object))
      return getMaximum(dynamic_cast<TH1*>(object));
    else if (dynamic_cast<THStack*>(object))
      return getMaximum(dynamic_cast<THStack*>(object));

    return std::numeric_limits<float>::lowest();
  }

  float getMinimum(TObject* object) {
    if (dynamic_cast<TH1*>(object))
      return getMinimum(dynamic_cast<TH1*>(object));
    else if (dynamic_cast<THStack*>(object))
      return getMinimum(dynamic_cast<THStack*>(object));

    return std::numeric_limits<float>::infinity();
  }

  void setMaximum(TObject* object, float minimum) {
    if (dynamic_cast<TH1*>(object))
      setMaximum(dynamic_cast<TH1*>(object), minimum);
    else if (dynamic_cast<THStack*>(object))
      setMaximum(dynamic_cast<THStack*>(object), minimum);
  }

  void setMinimum(TObject* object, float minimum) {
    if (dynamic_cast<TH1*>(object))
      setMinimum(dynamic_cast<TH1*>(object), minimum);
    else if (dynamic_cast<THStack*>(object))
      setMinimum(dynamic_cast<THStack*>(object), minimum);
  }

  void setRange(TObject* object, Plot& plot) {
    if (dynamic_cast<TH1*>(object))
      setRange(dynamic_cast<TH1*>(object), plot);
    else if (dynamic_cast<THStack*>(object))
      setRange(dynamic_cast<THStack*>(object)->GetHistogram(), plot);
  }
}
