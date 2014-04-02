#include <boost/filesystem.hpp>
#include <boost/algorithm/string/join.hpp>
#include <memory>

#include "yaml-cpp/yaml.h"

#include <TH1.h>
#include <THStack.h>
#include <TStyle.h>

#include <vector>
#include <string>
#include <glob.h>

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

  struct PlotStyle;
  class plotIt;
  struct Group;

  struct Summary {
    float n_events;
    float n_events_error;

    float efficiency;
    float efficiency_error;

    Summary() {
      n_events = n_events_error = efficiency = efficiency_error = 0;
    }

    void clear() {
      n_events = n_events_error = efficiency = efficiency_error = 0;
    }
  };

  struct Systematic {
    std::string path;
    TObject* object;

    Summary summary;
  };

  struct File {
    std::string path;

    // For MC and Signal
    float cross_section;
    float branching_ratio;
    float generated_events;

    // For Data
    float luminosity;

    std::shared_ptr<PlotStyle> plot_style;
    std::string group;

    Type type;

    TObject* object;

    std::vector<Systematic> systematics;

    int16_t order;
    Summary summary;
  };

  struct PlotStyle {

    // Style
    float marker_size;
    int16_t marker_color;
    int16_t marker_type;
    int16_t fill_color;
    int16_t fill_type;
    float line_width;
    int16_t line_color;
    int16_t line_type;
    std::string drawing_options;

    // Legend
    std::string legend;
    std::string legend_style;

    void loadFromYAML(YAML::Node& node, const File& file, plotIt& pIt);
  };

  struct Group {
    std::string name;
    std::shared_ptr<PlotStyle> plot_style;

    bool added;
  };

  struct Point {
    float x;
    float y;

    bool operator==(const Point& other) {
      return
        (fabs(x - other.x) < 1e-6) &&
        (fabs(y - other.y) < 1e-6);
    }
  };


  struct Plot {
    std::string name;

    bool normalized;
    bool log_y;

    std::string x_axis;
    std::string y_axis;

    // Axis range
    std::vector<float> x_axis_range;

    std::vector<std::string> save_extensions;

    bool show_ratio;
    bool fit_ratio = false;
    std::string fit_function = "pol1";
    std::string fit_legend;
    Point fit_legend_position = {0.20, 0.38};

    bool show_errors;

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
    float luminosity;
    float scale;

    // Systematics
    float luminosity_error_percent;

    int16_t error_fill_color;
    int16_t error_fill_style;

    int16_t ratio_fit_line_color = 46;
    int16_t ratio_fit_line_width = 1;
    int16_t ratio_fit_line_style = 1;
    int16_t ratio_fit_error_fill_color = 42;
    int16_t ratio_fit_error_fill_style = 1001;


    std::string title;
    std::string parsed_title;

    std::string root;

    Configuration() {
      width = height = 800;
      root = "./";
      luminosity = - 1;
      scale = 1.;
      luminosity_error_percent = 0;
      error_fill_color = 42;
      error_fill_style = 3154;
    }
  };

  class plotIt {
    public:
      plotIt(const fs::path& outputPath, const std::string& configFile);
      void plotAll();

      //void setLuminosity(float luminosity) {
        //m_luminosity = luminosity;
        //parseTitle();
      //}

      friend PlotStyle;

    private:
      void checkOrThrow(YAML::Node& node, const std::string& name, const std::string& file);
      void parseConfigurationFile(const std::string& file);
      int16_t loadColor(const YAML::Node& node);

      // Plot method
      bool plot(Plot& plot);

      bool plotTH1(TCanvas& c, Plot& plot);

      bool expandFiles();
      bool expandObjects(File& file, std::vector<Plot>& plots);
      bool loadObject(File& file, const Plot& plot);

      void setTHStyle(File& file);

      void addToLegend(TLegend& legend, Type type);

      void parseTitle();
      std::shared_ptr<PlotStyle> getPlotStyle(File& file);

      fs::path m_outputPath;

      std::vector<File> m_files;
      std::vector<Plot> m_plots;
      std::map<std::string, Group> m_groups;

      // Store objects in order to delete everything when drawing is done
      std::vector<std::shared_ptr<TObject>> m_temporaryObjects;

      // Temporary object living the whole runtime
      std::vector<std::shared_ptr<TObject>> m_temporaryObjectsRuntime;

      // Current style
      std::shared_ptr<TStyle> m_style;

      //float m_luminosity;

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
    //style->SetErrorX(0.);

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

    style->SetHatchesSpacing(1.3);
    style->SetHatchesLineWidth(1);

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
      if (plot.show_ratio)
        object->GetXaxis()->SetLabelSize(0);
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

  template<class T>
    float getMinimum(T* object) {
      return object->GetMinimum();
    }

  float getMinimum(TObject* object) {
    if (dynamic_cast<TH1*>(object))
      return getMinimum(dynamic_cast<TH1*>(object));
    else if (dynamic_cast<THStack*>(object))
      return getMinimum(dynamic_cast<THStack*>(object));

    return std::numeric_limits<float>::infinity();
  }

  template<class T>
    void setMaximum(T* object, float minimum) {
      object->SetMaximum(minimum);
    }

  void setMaximum(TObject* object, float minimum) {
    if (dynamic_cast<TH1*>(object))
      setMaximum(dynamic_cast<TH1*>(object), minimum);
    else if (dynamic_cast<THStack*>(object))
      setMaximum(dynamic_cast<THStack*>(object), minimum);
  }

  template<class T>
    void setMinimum(T* object, float minimum) {
      object->SetMinimum(minimum);
    }

  void setMinimum(TObject* object, float minimum) {
    if (dynamic_cast<TH1*>(object))
      setMinimum(dynamic_cast<TH1*>(object), minimum);
    else if (dynamic_cast<THStack*>(object))
      setMinimum(dynamic_cast<THStack*>(object), minimum);
  }

  template<class T>
    void setRange(T* object, Plot& plot) {
      object->GetXaxis()->SetRangeUser(plot.x_axis_range[0], plot.x_axis_range[1]);
    }

  void setRange(TObject* object, Plot& plot) {
    if (plot.x_axis_range.size() != 2)
      return;

    if (dynamic_cast<TH1*>(object))
      setRange(dynamic_cast<TH1*>(object), plot);
    else if (dynamic_cast<THStack*>(object))
      setRange(dynamic_cast<THStack*>(object)->GetHistogram(), plot);
  }

};

inline std::vector<std::string> glob(const std::string& pat) {
  glob_t glob_result;
  glob(pat.c_str(), GLOB_TILDE, NULL, &glob_result);

  std::vector<std::string> ret;
  for(unsigned int i = 0;i < glob_result.gl_pathc; ++i){
    ret.push_back(std::string(glob_result.gl_pathv[i]));
  }

  globfree(&glob_result);
  return ret;
}

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

  template<>
    struct convert<plotIt::Point> {
      static Node encode(const plotIt::Point& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);

        return node;
      }

      static bool decode(const Node& node, plotIt::Point& rhs) {
        if(!node.IsSequence() || node.size() != 2)
          return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();

        return true;
      }
    };
}

