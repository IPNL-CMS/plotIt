#include <boost/filesystem.hpp>
#include <boost/algorithm/string/join.hpp>
#include <memory>

#include "yaml-cpp/yaml.h"

#include <TH1.h>
#include <THStack.h>
#include <TStyle.h>

namespace YAML {
  class Node;
}

class TFile;
class TObject;
class TCanvas;
class TLegend;

namespace fs = boost::filesystem;

namespace plotIt {

  enum Type {
    MC,
    SIGNAL,
    DATA
  };

  struct File {
    std::string path;

    // For MC and Signal
    float cross_section;
    float branching_ratio;
    uint64_t generated_events;

    // For Data
    float luminosity;

    std::string legend;
    std::string legend_style;

    std::string drawing_options;

    Type type;

    // Style
    float marker_size;
    int16_t marker_color;
    int16_t marker_type;
    int16_t fill_color;
    int16_t fill_type;
    float line_width;
    int16_t line_color;
    int16_t line_type;

    TObject* object;
  };

  struct Plot {
    std::string name;

    bool normalized;

    std::string x_axis;
    std::string y_axis;

    std::vector<std::string> save_extensions;

    bool show_ratio;

    std::string inherits_from;

    uint16_t rebin;

    void print() {
      std::cout << "Plot '" << name << "'" << std::endl;
      std::cout << "\tx_axis: " << x_axis << std::endl;
      std::cout << "\ty_axis: " << y_axis << std::endl;
      std::cout << "\tshow_ratio: " << show_ratio << std::endl;
      std::cout << "\tinherits_from: " << inherits_from << std::endl;
      std::cout << "\tsave_extensions: " << boost::algorithm::join(save_extensions, ", ") << std::endl;
    }

    Plot Clone(const std::string& new_name) {
      Plot clone = *this;
      clone.name = new_name;

      return clone;
    }
  };

  struct Position {
    float x1;
    float y1;

    float x2;
    float y2;

    bool operator==(const Position& other) {
      return
        (fabs(x1 - other.x1) < 1e-6) &&
        (fabs(y1 - other.y1) < 1e-6) &&
        (fabs(x2 - other.x2) < 1e-6) &&
        (fabs(y2 - other.y2) < 1e-6);
    }
  };

  struct Legend {
    Position position;
  };

  struct Configuration {
    float width;
    float height;

    std::string title;
    std::string parsed_title;

    Configuration() {
      width = height = 800;
    }
  };

  class plotIt {
    public:
      plotIt(const fs::path& outputPath, const std::string& configFile);
      void plotAll();

      void setLuminosity(float luminosity) {
        m_luminosity = luminosity;
        parseTitle();
      }

    private:
      void checkOrThrow(YAML::Node& node, const std::string& name, const std::string& file);
      void parseConfigurationFile(const std::string& file);
      int16_t loadColor(const YAML::Node& node);

      // Plot method
      bool plot(Plot& plot);

      void plotTH1(TCanvas& c, Plot& plot);

      bool expandObjects(File& file, std::vector<Plot>& plots);
      bool loadObject(File& file, const Plot& plot);

      void setTHStyle(File& file);

      void addToLegend(TLegend& legend, Type type);

      void parseTitle();

      fs::path m_outputPath;

      std::vector<File> m_files;
      std::vector<Plot> m_plots;

      // Store objects in order to delete everything when drawing is done
      std::vector<std::shared_ptr<TObject>> m_temporaryObjects;

      // Temporary object living the whole runtime
      std::vector<std::shared_ptr<TObject>> m_temporaryObjectsRuntime;

      // Current style
      std::shared_ptr<TStyle> m_style;

      float m_luminosity;

      Legend m_legend;
      Configuration m_config;

      // For colors
      uint32_t m_colorIndex = 1000;
  };


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
    style->SetErrorX(0.);

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
    style->SetStatFont(42);
    style->SetStatFontSize(0.025);
    style->SetStatTextColor(1);
    style->SetStatFormat("6.4g");
    style->SetStatBorderSize(1);
    style->SetStatH(0.1);
    style->SetStatW(0.15);

    // Margins:
    style->SetPadTopMargin(0.05);
    style->SetPadBottomMargin(0.13);
    style->SetPadLeftMargin(0.16);
    style->SetPadRightMargin(0.05);

    // For the Global title:
    style->SetOptTitle(0);
    style->SetTitleFont(62);
    style->SetTitleColor(1);
    style->SetTitleTextColor(1);
    style->SetTitleFillColor(10);
    style->SetTitleFontSize(0.05);

    // For the axis titles:

    style->SetTitleColor(1, "XYZ");
    style->SetTitleFont(42, "XYZ");
    style->SetTitleSize(0.06, "XYZ");
    style->SetTitleXOffset(0.9);
    style->SetTitleYOffset(10);

    style->SetLabelColor(1, "XYZ");
    style->SetLabelFont(42, "XYZ");
    style->SetLabelOffset(0.007, "XYZ");
    style->SetLabelSize(0.05, "XYZ");

    style->SetAxisColor(1, "XYZ");
    style->SetStripDecimals(kTRUE);
    style->SetTickLength(0.03, "XYZ");
    style->SetNdivisions(510, "XYZ");
    style->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
    style->SetPadTickY(1);

    style->SetOptLogx(0);
    style->SetOptLogy(0);
    style->SetOptLogz(0);

    style->cd();

    return style;
  }

  template<class T>
    void setAxisTitles(T* object, Plot& plot) {
      if (plot.x_axis.length() > 0)
        object->GetXaxis()->SetTitle(plot.x_axis.c_str());
      if (plot.y_axis.length() > 0)
        object->GetYaxis()->SetTitle(plot.y_axis.c_str());

      object->GetYaxis()->SetTitleOffset(2);
    }

  void setAxisTitles(TObject* object, Plot& plot) {
    if (dynamic_cast<TH1*>(object))
      setAxisTitles(dynamic_cast<TH1*>(object), plot);
    else if (dynamic_cast<THStack*>(object))
      setAxisTitles(dynamic_cast<THStack*>(object), plot);
  }

  template<class T>
    float getMaximum(T* object) {
      return object->GetMaximum();
    }

  float getMaximum(TObject* object) {
    if (dynamic_cast<TH1*>(object))
      return getMaximum(dynamic_cast<TH1*>(object));
    else if (dynamic_cast<THStack*>(object))
      return getMaximum(dynamic_cast<THStack*>(object));

    return std::numeric_limits<float>::lowest();
  }

};

namespace YAML {
  template<>
    struct convert<plotIt::Position> {
      static Node encode(const plotIt::Position& rhs) {
        Node node;
        node.push_back(rhs.x1);
        node.push_back(rhs.y1);
        node.push_back(rhs.x1);
        node.push_back(rhs.y1);

        return node;
      }

      static bool decode(const Node& node, plotIt::Position& rhs) {
        if(!node.IsSequence() || node.size() != 4)
          return false;

        rhs.x1 = node[0].as<float>();
        rhs.y1 = node[1].as<float>();
        rhs.x2 = node[2].as<float>();
        rhs.y2 = node[2].as<float>();

        return true;
      }
    };
}

