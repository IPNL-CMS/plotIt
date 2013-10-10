#include "plotIt.h"

// For fnmatch()
#include <fnmatch.h>

#include <TH1.h>
#include <TStyle.h>
#include <TList.h>
#include <TCollection.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TKey.h>
#include <THStack.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TColor.h>

#include <vector>
#include <map>
#include <fstream>
#include <limits>
#include <TLorentzVector.h>

#include "tclap/CmdLine.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

// Load libdpm at startup, on order to be sure that rfio files are working
#include <dlfcn.h>
struct Dummy
{
  Dummy()
  {
    dlopen("libdpm.so", RTLD_NOW|RTLD_GLOBAL);
  }
};
static Dummy foo;

namespace plotIt {

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

  plotIt::plotIt(const fs::path& outputPath, const std::string& configFile):
    m_outputPath(outputPath) {

      m_style.reset(createStyle());
      parseConfigurationFile(configFile);
    }

  int16_t plotIt::loadColor(const YAML::Node& node) {
    std::string value = node.as<std::string>();
    if (value.length() > 1 && value[0] == '#' && ((value.length() == 7) || (value.length() == 9))) {
      // RGB Color
      std::string c = value.substr(1);
      // Convert to int with hexadecimal base
      uint32_t color = 0;
      std::stringstream ss;
      ss << std::hex << c;
      ss >> color;

      float a = 1;
      if (color > 0xffffff) {
        a = (color >> 24) / 255.0;
      }

      float r = ((color >> 16) & 0xff) / 255.0;
      float g = ((color >> 8) & 0xff) / 255.0;
      float b = ((color) & 0xff) / 255.0;

      // Create new color
      m_temporaryObjectsRuntime.push_back(std::make_shared<TColor>(m_colorIndex++, r, g, b, "", a));

      return m_colorIndex - 1;
    } else {
      return node.as<int16_t>();
    }
  }

  void plotIt::parseConfigurationFile(const std::string& file) {
    YAML::Node f = YAML::LoadFile(file);

    if (! f["files"]) {
      throw YAML::ParserException(YAML::Mark::null_mark(), "Your configuration file must have a 'files' list");
    }

    YAML::Node files = f["files"];

    for (YAML::const_iterator it = files.begin(); it != files.end(); ++it) {
      File file;

      file.path = it->first.as<std::string>();

      YAML::Node node = it->second;

      if (node["type"]) {
        std::string type = node["type"].as<std::string>();
        if (type == "signal")
          file.type = SIGNAL;
        else if (type == "data")
          file.type = DATA;
        else
          file.type = MC;
      } else
        file.type = MC;

      if (node["cross-section"])
        file.cross_section = node["cross-section"].as<float>();
      else
        file.cross_section = 1;

      if (node["branching-ratio"])
        file.branching_ratio = node["branching-ratio"].as<float>();
      else
        file.branching_ratio = 1;

      if (node["generated-events"])
        file.generated_events = node["generated-events"].as<uint64_t>();
      else
        file.generated_events = 1;

      if (node["legend"])
        file.legend = node["legend"].as<std::string>();

      if (file.type == MC)
        file.legend_style = "lf";
      else if (file.type == SIGNAL)
        file.legend_style = "l";
      else if (file.type == DATA)
        file.legend_style = "p";

      if (node["legend-style"])
        file.legend_style = node["legend-style"].as<std::string>();

      if (node["drawing-options"])
        file.drawing_options = node["drawing-options"].as<std::string>();
      else {
        if (file.type == MC || file.type == SIGNAL)
          file.drawing_options = "hist";
        else if (file.type == DATA)
          file.drawing_options = "P";
      }

      file.marker_size = -1;
      file.marker_color = -1;
      file.marker_type = -1;

      file.fill_color = -1;
      file.fill_type = -1;

      file.line_color = -1;
      file.line_type = -1;

      if (file.type == MC) {
        file.fill_color = 1;
        file.fill_type = 1001;
        file.line_width = 0;
      } else if (file.type == SIGNAL) {
        file.fill_type = 0;
        file.line_color = 1;
        file.line_width = 1;
        file.line_type = 2;
      } else {
        file.marker_size = 1;
        file.marker_color = 1;
        file.marker_type = 20;
      }

      if (node["fill-color"])
        file.fill_color = loadColor(node["fill-color"]);

      if (node["fill-type"])
        file.fill_type = node["fill-type"].as<int16_t>();

      if (node["line-color"])
        file.line_color = loadColor(node["line-color"]);

      if (node["line-type"])
        file.line_type = node["line-type"].as<int16_t>();

      if (node["line-width"])
        file.line_width = node["line-width"].as<float>();

      if (node["marker-color"])
        file.marker_color = loadColor(node["marker-color"]);

      if (node["marker-type"])
        file.marker_type = node["marker-type"].as<int16_t>();

      if (node["marker-size"])
        file.marker_size = node["marker-size"].as<float>();

      m_files.push_back(file);
    }

    if (! f["plots"]) {
      throw YAML::ParserException(YAML::Mark::null_mark(), "You must specify at least one plot in your configuration file");
    }

    YAML::Node plots = f["plots"];
    for (YAML::const_iterator it = plots.begin(); it != plots.end(); ++it) {
      Plot plot;

      plot.name = it->first.as<std::string>();

      YAML::Node node = it->second;
      if (node["x-axis"])
        plot.x_axis = node["x-axis"].as<std::string>();

      if (node["y-axis"])
        plot.y_axis = node["y-axis"].as<std::string>();

      if (node["normalized"])
        plot.normalized = node["normalized"].as<bool>();

      if (node["save-extensions"])
        plot.save_extensions = node["save-extensions"].as<std::vector<std::string>>();
      else
        plot.save_extensions = {"pdf"};

      if (node["show-ratio"])
        plot.show_ratio = node["show-ratio"].as<bool>();
      else
        plot.show_ratio = false;

      if (node["inherits-from"])
        plot.inherits_from = node["inherits-from"].as<std::string>();
      else
        plot.inherits_from = "TH1";

      if (node["rebin"])
        plot.rebin = node["rebin"].as<uint16_t>();
      else
        plot.rebin = 1;

      m_plots.push_back(plot);

      plot.print();
    }

    m_legend.position.x1 = 0.6;
    m_legend.position.y1 = 0.6;

    m_legend.position.x2 = 0.9;
    m_legend.position.y2 = 0.9;
    if (f["legend"]) {
      YAML::Node node = f["legend"];

      if (node["position"])
        m_legend.position = node["position"].as<Position>();
    }

    if (f["configuration"]) {
      YAML::Node node = f["configuration"];

      if (node["width"])
        m_config.width = node["width"].as<float>();

      if (node["height"])
        m_config.height = node["height"].as<float>();

      if (node["title"])
        m_config.title = node["title"].as<std::string>();
    }

    parseTitle();
  }

  void plotIt::parseTitle() {

    m_config.parsed_title = m_config.title;

    float lumi = m_luminosity / 1000.;

    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << lumi;
    std::string lumiStr = out.str();

    boost::algorithm::replace_all(m_config.parsed_title, "%lumi%", lumiStr);
  }

  void plotIt::setTHStyle(File& file) {
    TH1* h = dynamic_cast<TH1*>(file.object);

    if (file.fill_color != -1)
      h->SetFillColor(file.fill_color);

    if (file.fill_type != -1)
      h->SetFillStyle(file.fill_type);

    if (file.line_color != -1)
      h->SetLineColor(file.line_color);

    if (file.line_width != -1)
      h->SetLineWidth(file.line_width);

    if (file.line_type != -1)
      h->SetLineStyle(file.line_type);

    if (file.marker_size != -1)
      h->SetMarkerSize(file.marker_size);

    if (file.marker_color != -1)
      h->SetMarkerColor(file.marker_color);

    if (file.marker_type != -1)
      h->SetMarkerStyle(file.marker_type);

    if (file.type == MC && file.line_color == -1 && file.fill_color != -1)
      h->SetLineColor(file.fill_color);
  }

  void plotIt::addToLegend(TLegend& legend, Type type) {
    for (File& file: m_files) {
      if (file.type == type && file.legend.length() > 0) {
        legend.AddEntry(file.object, file.legend.c_str(), file.legend_style.c_str());
      }
    }
  }

  bool plotIt::plot(Plot& plot) {
    std::cout << "Plotting '" << plot.name << "'" << std::endl;

    bool hasLegend = false;
    // Open all files, and find histogram in each
    for (File& file: m_files) {
      if (! loadObject(file, plot)) {
        return false;
      }

      hasLegend |= file.legend.length() > 0;
    }

    // Create canvas
    TCanvas c("canvas", "canvas", m_config.width, m_config.height);

    if (m_files[0].object->InheritsFrom("TH1")) {
      plotTH1(c, plot);
    }

    // Build legend
    TLegend legend(m_legend.position.x1, m_legend.position.y1, m_legend.position.x2, m_legend.position.y2);
    legend.SetTextFont(42);
    legend.SetFillStyle(0);
    legend.SetBorderSize(0);

    addToLegend(legend, MC);
    addToLegend(legend, SIGNAL);
    addToLegend(legend, DATA);

    legend.Draw();

    // Title
    if (m_config.title.length() > 0) {
      std::shared_ptr<TPaveText> pt = std::make_shared<TPaveText>(0.15, 0.955, 0.95, 0.995, "brNDC");
      m_temporaryObjects.push_back(pt);

      pt->SetFillStyle(0);
      pt->SetBorderSize(0);
      pt->SetTextFont(42);
      pt->SetTextAlign(32);

      pt->AddText(m_config.parsed_title.c_str());
      pt->Draw();
    }

    fs::path outputName = m_outputPath / plot.name;

    for (const std::string& extension: plot.save_extensions) {
      fs::path outputNameWithExtension = outputName.replace_extension(extension);

      c.SaveAs(outputNameWithExtension.string().c_str());
    }

    // Close all opened files
    m_temporaryObjects.clear();
    return true;
  }

  void plotIt::plotTH1(TCanvas& c, Plot& plot) {
    c.cd();

    // Rescale and style histograms
    for (File& file: m_files) {
      setTHStyle(file);

      TH1* h = dynamic_cast<TH1*>(file.object);

      if (file.type != DATA) {
        float factor = m_luminosity * file.cross_section * file.branching_ratio / file.generated_events;
        h->Scale(factor);
      }

      h->Rebin(plot.rebin);
    }

    // Build a THStack for MC files and a vector for signal
    float mcIntegral = 0;
    std::shared_ptr<THStack> mc_stack = std::make_shared<THStack>("mc_stack", "mc_stack");
    std::vector<File> signals;
    std::vector<File> datas;
    for (File& file: m_files) {
      if (file.type == MC) {
        mc_stack->Add(dynamic_cast<TH1*>(file.object), file.drawing_options.c_str());
        mcIntegral += dynamic_cast<TH1*>(file.object)->Integral();
      } else if (file.type == SIGNAL) {
        signals.push_back(file);
      } else if (file.type == DATA) {
        datas.push_back(file);
      }
    }

    std::shared_ptr<TH1> data;
    std::string data_drawing_options;
    if (datas.size()) {
      for (File& d: datas) {
        if (! data.get()) {
          data.reset(dynamic_cast<TH1*>(d.object->Clone()));
          data->SetDirectory(nullptr);
          data_drawing_options = d.drawing_options;
        }
        else {
          data->Add(dynamic_cast<TH1*>(d.object));
        }
      }
    }

    if (plot.normalized) {
      // Normalized each plot
      for (File& file: m_files) {
        TH1* h = dynamic_cast<TH1*>(file.object);
        if (file.type == MC) {
          h->Scale(1. / mcIntegral);
        } else {
          h->Scale(1. / h->Integral());
        }
      }

      if (data.get()) {
        data->Scale(1. / data->Integral());
      }
    }

    // Store all the histograms to draw, and find the one with the highest maximum
    std::vector<
      std::pair<TObject*, std::string>
      > toDraw = { std::make_pair(mc_stack.get(), ""), std::make_pair(data.get(), data_drawing_options) };
    for (File& signal: signals) {
      toDraw.push_back(std::make_pair(signal.object, signal.drawing_options));
    }

    // Remove NULL items
    toDraw.erase(
        std::remove_if(toDraw.begin(), toDraw.end(), [](std::pair<TObject*, std::string>& element) {
          return element.first == nullptr;
        })
      );

    // Sort files
    std::sort(toDraw.begin(), toDraw.end(), [](std::pair<TObject*, std::string> a, std::pair<TObject*, std::string> b) {
        return getMaximum(a.first) > getMaximum(b.first);
      });

    toDraw[0].first->Draw(toDraw[0].second.c_str());
    // Set x and y axis titles
    setAxisTitles(toDraw[0].first, plot);

    // First, draw MC
    mc_stack->Draw("same");
    m_temporaryObjects.push_back(mc_stack);

    // Then signal
    for (File& signal: signals) {
      std::string options = signal.drawing_options + " same";
      signal.object->Draw(options.c_str());
    }

    // And finally data
    if (data.get()) {
      data_drawing_options += " same";
      data->Draw(data_drawing_options.c_str());
      m_temporaryObjects.push_back(data);
    }
  }

  void plotIt::plotAll() {
    // First, explode plots to match all glob patterns
    std::vector<Plot> plots;
    if (! expandObjects(m_files[0], plots)) {
      return;
    }

    for (Plot& plot: plots) {
      plotIt::plot(plot);
    }
  }

  bool plotIt::loadObject(File& file, const Plot& plot) {

    file.object = nullptr;

    std::shared_ptr<TFile> input(TFile::Open(file.path.c_str()));
    if (! input.get())
      return false;

    TObject* obj = input->Get(plot.name.c_str());

    if (obj) {
      m_temporaryObjects.push_back(input);
      file.object = obj;

      return true;
    }

    // Should not be possible!
    std::cout << "Error: object '" << plot.name << "' inheriting from '" << plot.inherits_from << "' not found in file '" << file.path << "'" << std::endl;
    return false;
  }

  /**
   * Open 'file', and expand all plots
   */
  bool plotIt::expandObjects(File& file, std::vector<Plot>& plots) {
    file.object = nullptr;
    plots.clear();

    std::shared_ptr<TFile> input(TFile::Open(file.path.c_str()));
    if (! input.get())
      return false;

    TIter keys(input->GetListOfKeys());

    for (Plot& plot: m_plots) {
      TKey* key;
      TObject* obj;
      bool match = false;

      keys.Reset();
      while ((key = static_cast<TKey*>(keys()))) {
        obj = key->ReadObj();
        if (! obj->InheritsFrom(plot.inherits_from.c_str()))
          continue;

        // Check name
        if (fnmatch(plot.name.c_str(), obj->GetName(), FNM_CASEFOLD) == 0) {
          // Got it!
          match = true;
          plots.push_back(plot.Clone(obj->GetName()));
        }
      }

      if (! match) {
        std::cout << "Warning: object '" << plot.name << "' inheriting from '" << plot.inherits_from << "' does not match something in file '" << file.path << "'" << std::endl;
      }
    }

    if (!plots.size()) {
      std::cout << "Error: no plots found in file '" << file.path << "'" << std::endl;
      return false;
    }

    return true;
  }

  TFile* loadHistograms(const std::string& file, std::map<std::string, TH1*>& histograms) {
    TFile* f = TFile::Open(file.c_str());

    TList *list = f->GetListOfKeys();
    TIter iter(list);

    TKey* key;
    TObject* obj;

    while ((key = static_cast<TKey*>(iter()))) {
      obj = key->ReadObj();
      if (! obj->InheritsFrom("TH1"))
        continue;

      TH1* hist = static_cast<TH1*>(obj);
      histograms[hist->GetName()] = hist;
    }

    return f;
  }

}

int main(int argc, char** argv) {

  try {

    TCLAP::CmdLine cmd("Plot histograms", ' ', "0.1");

    TCLAP::ValueArg<std::string> outputFolderArg("o", "output-folder", "output folder", true, "", "string", cmd);

    //TCLAP::SwitchArg dataArg("", "data", "Is this data?", false);
    //TCLAP::SwitchArg mcArg("", "mc", "Is this mc?", false);

    //cmd.xorAdd(dataArg, mcArg);

    TCLAP::SwitchArg semimuArg("", "semimu", "Is this semi-mu channel?", false);
    TCLAP::SwitchArg semieArg("", "semie", "Is this semi-e channel?", false);

    cmd.xorAdd(semimuArg, semieArg);

    TCLAP::ValueArg<int> btagArg("", "b-tag", "Number of b-tagged jet to require", true, 2, "int", cmd);

    TCLAP::ValueArg<float> lumiArg("", "lumi", "Luminosity", false, 19700, "int", cmd);

    //TCLAP::ValueArg<std::string> configArg("", "config-path", "Path to the configuration files", false, "./config", "int", cmd);

    TCLAP::UnlabeledValueArg<std::string> configFileArg("configFile", "configuration file", true, "", "string", cmd);

    cmd.parse(argc, argv);

    //bool isData = dataArg.isSet();

    fs::path outputPath(outputFolderArg.getValue());

    if (! fs::exists(outputPath)) {
      std::cout << "Error: output path " << outputPath << " does not exist" << std::endl;
      return 1;
    }

    plotIt::plotIt p(outputPath, configFileArg.getValue());
    p.setLuminosity(lumiArg.getValue());

    p.plotAll();

  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }

  return 0;
}
