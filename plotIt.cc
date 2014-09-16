#include "plotIt.h"

// For fnmatch()
#include <fnmatch.h>

#include <TH1.h>
#include <TF1.h>
#include <TStyle.h>
#include <TList.h>
#include <TCollection.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TKey.h>
#include <THStack.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TPaveText.h>
#include <TColor.h>
#include <TVirtualFitter.h>

#include <iomanip>
#include <vector>
#include <map>
#include <fstream>
#include <limits>
#include <TLorentzVector.h>

#include "tclap/CmdLine.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;
using std::setw;

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

  plotIt::plotIt(const fs::path& outputPath, const std::string& configFile):
    m_outputPath(outputPath) {

      gErrorIgnoreLevel = kError;
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

    if (f["configuration"]) {
      YAML::Node node = f["configuration"];

      if (node["width"])
        m_config.width = node["width"].as<float>();

      if (node["height"])
        m_config.height = node["height"].as<float>();

      if (node["title"])
        m_config.title = node["title"].as<std::string>();

      if (node["root"])
        m_config.root = node["root"].as<std::string>();

      if (node["scale"])
        m_config.scale = node["scale"].as<float>();

      if (node["luminosity"])
        m_config.luminosity = node["luminosity"].as<float>();
      else {
        throw YAML::ParserException(YAML::Mark::null_mark(), "'configuration' block is missing luminosity");
      }

      if (node["luminosity-error"])
        m_config.luminosity_error_percent = node["luminosity-error"].as<float>();

      if (node["error-fill-color"])
        m_config.error_fill_color = loadColor(node["error-fill-color"]);

      if (node["error-fill-style"])
        m_config.error_fill_style = loadColor(node["error-fill-style"]);

      if (node["ratio-fit-line-style"])
        m_config.ratio_fit_line_style = node["ratio-fit-line-style"].as<int16_t>();

      if (node["ratio-fit-line-width"])
        m_config.ratio_fit_line_width = node["ratio-fit-line-width"].as<int16_t>();

      if (node["ratio-fit-line-color"])
        m_config.ratio_fit_line_color = loadColor(node["ratio-fit-line-color"]);

      if (node["ratio-fit-error-fill-style"])
        m_config.ratio_fit_error_fill_style = node["ratio-fit-error-fill-style"].as<int16_t>();

      if (node["ratio-fit-error-fill-color"])
        m_config.ratio_fit_error_fill_color = loadColor(node["ratio-fit-error-fill-color"]);

    }

    YAML::Node groups = f["groups"];

    for (YAML::const_iterator it = groups.begin(); it != groups.end(); ++it) {
      Group group;

      group.name = it->first.as<std::string>();

      YAML::Node node = it->second;

      group.plot_style = std::make_shared<PlotStyle>();
      group.plot_style->loadFromYAML(node, File(), *this);

      m_groups[group.name] = group;
    }


    YAML::Node files = f["files"];

    for (YAML::const_iterator it = files.begin(); it != files.end(); ++it) {
      File file;

      file.path = it->first.as<std::string>();
      fs::path root = fs::path(m_config.root);
      fs::path path = fs::path(file.path);
      file.path = (root / path).string();

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

      if (node["scale"])
        file.scale = node["scale"].as<float>();
      else
        file.scale = 1;

      if (node["cross-section"])
        file.cross_section = node["cross-section"].as<float>();
      else
        file.cross_section = 1;

      if (node["branching-ratio"])
        file.branching_ratio = node["branching-ratio"].as<float>();
      else
        file.branching_ratio = 1;

      if (node["generated-events"])
        file.generated_events = node["generated-events"].as<float>();
      else
        file.generated_events = 1.;

      file.order = std::numeric_limits<int16_t>::min();
      if (node["order"])
        file.order = node["order"].as<int16_t>();

      if (node["group"]) {
        file.group = node["group"].as<std::string>();

        if (! m_groups.count(file.group)) {
          file.group = "";
        }
      }

      if (node["systematics"]) {

        const auto& addSystematic = [&root, &file](const std::string& systematics) {
          fs::path path = fs::path(systematics);
          std::string fullPath = (root / path).string();

          if (boost::filesystem::exists(fullPath)) {
            Systematic s;
            s.path = fullPath;

            file.systematics.push_back(s);
          } else {
            std::cerr << "Warning: systematics file '" << systematics << "' not found." << std::endl;
          }
        };

        const YAML::Node& syst = node["systematics"];
        if (syst.IsSequence()) {
          for (const auto& it: syst) {
            addSystematic(it.as<std::string>());
          }
        } else {
          addSystematic(syst.as<std::string>());
        }
      }

      if (file.group.length() == 0) {
        file.plot_style = std::make_shared<PlotStyle>();
        file.plot_style->loadFromYAML(node, file, *this);
      }

      m_files.push_back(file);
    }

    std::sort(m_files.begin(), m_files.end(), [](const File& a, const File& b) {
      return a.order < b.order;
     });

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

      plot.normalized = false;
      if (node["normalized"])
        plot.normalized = node["normalized"].as<bool>();

      plot.log_y = false;
      if (node["log-y"])
        plot.log_y = node["log-y"].as<bool>();

      if (node["save-extensions"])
        plot.save_extensions = node["save-extensions"].as<std::vector<std::string>>();
      else
        plot.save_extensions = {"pdf"};

      if (node["show-ratio"])
        plot.show_ratio = node["show-ratio"].as<bool>();
      else
        plot.show_ratio = false;

      if (node["fit-ratio"])
        plot.fit_ratio = node["fit-ratio"].as<bool>();

      if (node["fit-function"])
        plot.fit_function = node["fit-function"].as<std::string>();

      if (node["fit-legend"])
        plot.fit_legend = node["fit-legend"].as<std::string>();

      if (node["fit-legend-position"])
        plot.fit_legend_position = node["fit-legend-position"].as<Point>();

      if (node["show-errors"])
        plot.show_errors = node["show-errors"].as<bool>();
      else
        plot.show_errors = false;

      if (node["x-axis-range"])
        plot.x_axis_range = node["x-axis-range"].as<std::vector<float>>();

      if (node["y-axis-range"])
        plot.y_axis_range = node["y-axis-range"].as<std::vector<float>>();

      if (node["inherits-from"])
        plot.inherits_from = node["inherits-from"].as<std::string>();
      else
        plot.inherits_from = "TH1";

      if (node["rebin"])
        plot.rebin = node["rebin"].as<uint16_t>();
      else
        plot.rebin = 1;

      m_plots.push_back(plot);
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

    parseTitle();
  }

  void plotIt::parseTitle() {

    m_config.parsed_title = m_config.title;

    float lumi = m_config.luminosity / 1000.;

    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << lumi;
    std::string lumiStr = out.str();

    boost::algorithm::replace_all(m_config.parsed_title, "%lumi%", lumiStr);
  }

  void plotIt::setTHStyle(File& file) {
    TH1* h = dynamic_cast<TH1*>(file.object);

    std::shared_ptr<PlotStyle> style = getPlotStyle(file);

    if (style->fill_color != -1)
      h->SetFillColor(style->fill_color);

    if (style->fill_type != -1)
      h->SetFillStyle(style->fill_type);

    if (style->line_color != -1)
      h->SetLineColor(style->line_color);

    if (style->line_width != -1)
      h->SetLineWidth(style->line_width);

    if (style->line_type != -1)
      h->SetLineStyle(style->line_type);

    if (style->marker_size != -1)
      h->SetMarkerSize(style->marker_size);

    if (style->marker_color != -1)
      h->SetMarkerColor(style->marker_color);

    if (style->marker_type != -1)
      h->SetMarkerStyle(style->marker_type);

    if (file.type == MC && style->line_color == -1 && style->fill_color != -1)
      h->SetLineColor(style->fill_color);
  }

  void plotIt::addToLegend(TLegend& legend, Type type) {
    for (File& file: m_files) {
      if (file.type == type) {
        if (file.group.length() > 0 && m_groups.count(file.group) &&
            !m_groups[file.group].added && m_groups[file.group].plot_style->legend.length() > 0) {
          legend.AddEntry(file.object, m_groups[file.group].plot_style->legend.c_str(), m_groups[file.group].plot_style->legend_style.c_str());
          m_groups[file.group].added = true;
        } else if (file.plot_style.get() && file.plot_style->legend.length() > 0) {
          legend.AddEntry(file.object, file.plot_style->legend.c_str(), file.plot_style->legend_style.c_str());
        }
      }
    }
  }

  bool plotIt::plot(Plot& plot) {
    std::cout << "Plotting '" << plot.name << "'" << std::endl;

    bool hasMC = false;
    bool hasData = false;
    bool hasSignal = false;
    bool hasLegend = false;
    // Open all files, and find histogram in each
    for (File& file: m_files) {
      if (! loadObject(file, plot)) {
        return false;
      }

      hasLegend |= getPlotStyle(file)->legend.length() > 0;
      hasData |= file.type == DATA;
      hasMC |= file.type == MC;
      hasSignal |= file.type == SIGNAL;
    }

    // Create canvas
    TCanvas c("canvas", "canvas", m_config.width, m_config.height);

    bool success = false;
    if (m_files[0].object->InheritsFrom("TH1")) {
      success = plotTH1(c, plot);
    }

    auto printSummary = [&](Type type) {
      float sum_n_events = 0;
      float sum_n_events_error = 0;

      printf("%50s%18s ± %11s%15s%19s  ± %20s\n", " ", "N", u8"ΔN", " ", u8"ε", u8"Δε");
      for (File& file: m_files) {
        if (file.type == type) {
          fs::path path(file.path);
          printf("%50s%18.2f ± %10.2f%15s%18.5f%% ± %18.5f%%\n", path.stem().c_str(), file.summary.n_events, file.summary.n_events_error, " ", file.summary.efficiency * 100, file.summary.efficiency_error * 100);

          sum_n_events += file.summary.n_events;
          sum_n_events_error += file.summary.n_events_error * file.summary.n_events_error;
        }
      }

      if (sum_n_events) {
        float systematics = 0;
        if (type == MC && m_config.luminosity_error_percent > 0) {
          std::cout << "------------------------------------------" << std::endl;
          std::cout << "Systematic uncertainties" << std::endl;
          printf("%50s%18s ± %10.2f\n", "Luminosity", " ", sum_n_events * m_config.luminosity_error_percent);
          systematics = sum_n_events * m_config.luminosity_error_percent;
        }
        for (File& file: m_files) {
          if (file.type == type) {
            for (Systematic& s: file.systematics) {
              fs::path path(s.path);
              printf("%50s%18s ± %10.2f\n", path.stem().c_str(), " ", s.summary.n_events_error);

              sum_n_events_error += s.summary.n_events_error * s.summary.n_events_error;
            }
          }
        }
        std::cout << "------------------------------------------" << std::endl;
        printf("%50s%18.2f ± %10.2f\n", " ", sum_n_events, sqrt(sum_n_events_error + systematics * systematics));
      }
    };

    if (! success)
      return false;

    std::cout << "Summary: " << std::endl;

    if (hasData) {
     std::cout << "Data" << std::endl;
     printSummary(DATA);
     std::cout << std::endl;
    }

    if (hasMC) {
     std::cout << "MC: " << std::endl;
     printSummary(MC);
     std::cout << std::endl;
    }

    if (hasSignal) {
     std::cout << std::endl << "Signal: " << std::endl;
     printSummary(SIGNAL);
     std::cout << std::endl;
    }

    if (plot.log_y)
      c.SetLogy();

    // Build legend
    TLegend legend(m_legend.position.x1, m_legend.position.y1, m_legend.position.x2, m_legend.position.y2);
    legend.SetTextFont(42);
    legend.SetFillStyle(0);
    legend.SetBorderSize(0);

    addToLegend(legend, MC);
    addToLegend(legend, SIGNAL);
    addToLegend(legend, DATA);

    if (plot.show_errors) {
      TLegendEntry* entry = legend.AddEntry("errors", "Uncertainties", "f");
      entry->SetLineWidth(0);
      entry->SetLineColor(m_config.error_fill_color);
      entry->SetFillStyle(m_config.error_fill_style);
      entry->SetFillColor(m_config.error_fill_color);
    }

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

    // Reset groups
    for (auto& group: m_groups) {
      group.second.added = false;
    }

    // Clear summary
    for (auto& file: m_files) {
      file.summary.clear();
    }

    return true;
  }

  bool plotIt::plotTH1(TCanvas& c, Plot& plot) {
    c.cd();

    // Rescale and style histograms
    for (File& file: m_files) {
      setTHStyle(file);

      TH1* h = dynamic_cast<TH1*>(file.object);

      if (file.type != DATA) {
        float factor = m_config.scale * m_config.luminosity * file.scale * file.cross_section * file.branching_ratio / file.generated_events;

        float n_entries = h->Integral();
        file.summary.efficiency = n_entries / file.generated_events;
        file.summary.efficiency_error = sqrt( (file.summary.efficiency * (1 - file.summary.efficiency)) / file.generated_events );

        file.summary.n_events = n_entries * factor;
        file.summary.n_events_error = m_config.scale * m_config.luminosity * file.cross_section * file.branching_ratio * file.summary.efficiency_error;

        h->Scale(factor);
      } else {
        file.summary.n_events = h->Integral();
        file.summary.n_events_error = 0;
      }

      h->Rebin(plot.rebin);

      for (Systematic& s: file.systematics) {
        TH1* syst = static_cast<TH1*>(s.object);
        syst->Rebin(plot.rebin);
      }
    }

    // Build a THStack for MC files and a vector for signal
    float mcWeight = 0;
    std::shared_ptr<THStack> mc_stack = std::make_shared<THStack>("mc_stack", "mc_stack");
    std::shared_ptr<TH1> mc_histo_stat_only;
    std::shared_ptr<TH1> mc_histo_syst_only;
    std::shared_ptr<TH1> mc_histo_stat_syst;

    std::shared_ptr<TH1> h_data;
    std::string data_drawing_options;

    std::vector<File> signal_files;

    for (File& file: m_files) {
      if (file.type == MC) {

        TH1* nominal = dynamic_cast<TH1*>(file.object);

        mc_stack->Add(nominal, getPlotStyle(file)->drawing_options.c_str());
        if (mc_histo_stat_only.get()) {
          mc_histo_stat_only->Add(nominal);
        } else {
          mc_histo_stat_only.reset( dynamic_cast<TH1*>(nominal->Clone()) );
          mc_histo_stat_only->SetDirectory(nullptr);
        }
        mcWeight += nominal->GetSumOfWeights();

      } else if (file.type == SIGNAL) {
        signal_files.push_back(file);
      } else if (file.type == DATA) {
        if (! h_data.get()) {
          h_data.reset(dynamic_cast<TH1*>(file.object->Clone()));
          h_data->SetDirectory(nullptr);
          data_drawing_options += getPlotStyle(file)->drawing_options;
        } else {
          h_data->Add(dynamic_cast<TH1*>(file.object));
        }
      }
    }

    if ((h_data.get()) && !h_data->GetSumOfWeights())
      h_data.reset();

    if ((mc_histo_stat_only.get() && !mc_histo_stat_only->GetSumOfWeights())) {
      mc_histo_stat_only.reset();
      mc_stack.reset();
    }

    if (plot.normalized) {
      // Normalized each plot
      for (File& file: m_files) {
        TH1* h = dynamic_cast<TH1*>(file.object);
        if (file.type == MC) {
          h->Scale(1. / fabs(mcWeight));
        } else if (file.type == SIGNAL) {
          h->Scale(1. / fabs(h->GetSumOfWeights()));
        }
      }

      if (h_data.get()) {
        h_data->Scale(1. / h_data->GetSumOfWeights());
      }
    }

    if (mc_histo_stat_only.get()) {
      mc_histo_syst_only.reset(static_cast<TH1*>(mc_histo_stat_only->Clone()));
      mc_histo_syst_only->SetDirectory(nullptr);
      mc_histo_stat_syst.reset(static_cast<TH1*>(mc_histo_stat_only->Clone()));
      mc_histo_stat_syst->SetDirectory(nullptr);

      // Clear statistical errors
      for (uint32_t i = 1; i <= (uint32_t) mc_histo_syst_only->GetNbinsX(); i++) {
        mc_histo_syst_only->SetBinError(i, 0);
      }
    }

    if (mc_histo_syst_only.get() && plot.show_errors) {
      if (m_config.luminosity_error_percent > 0) {
        // Loop over all bins, and add lumi error
        for (uint32_t i = 1; i <= (uint32_t) mc_histo_syst_only->GetNbinsX(); i++) {
          float error = mc_histo_syst_only->GetBinError(i);
          float entries = mc_histo_syst_only->GetBinContent(i);
          float lumi_error = entries * m_config.luminosity_error_percent;

          mc_histo_syst_only->SetBinError(i, std::sqrt(error * error + lumi_error * lumi_error));
        }
      }

      // Check if systematic histogram are attached, and add them to the plot
      for (File& file: m_files) {
        if (file.type != MC || file.systematics.size() == 0)
          continue;

        TH1* nominal = dynamic_cast<TH1*>(file.object);

        for (Systematic& syst: file.systematics) {
          // This histogram should contains syst errors
          // in percent
          float total_syst_error = 0;
          TH1* h = dynamic_cast<TH1*>(syst.object);
          for (uint32_t i = 1; i <= (uint32_t) mc_histo_syst_only->GetNbinsX(); i++) {
            float total_error = mc_histo_syst_only->GetBinError(i);
            float syst_error_percent = h->GetBinError(i);
            float nominal_value = nominal->GetBinContent(i);
            float syst_error = nominal_value * syst_error_percent;
            total_syst_error += syst_error;

            mc_histo_syst_only->SetBinError(i, std::sqrt(total_error * total_error + syst_error * syst_error));
          }

          syst.summary.n_events = file.summary.n_events;
          syst.summary.n_events_error = total_syst_error;
        }
      }

      // Propagate syst errors to the stat + syst histogram
      for (uint32_t i = 1; i <= (uint32_t) mc_histo_syst_only->GetNbinsX(); i++) {
        float syst_error = mc_histo_syst_only->GetBinError(i);
        float stat_error = mc_histo_stat_only->GetBinError(i);
        mc_histo_stat_syst->SetBinError(i, std::sqrt(syst_error * syst_error + stat_error * stat_error));
      }
    }

    // Store all the histograms to draw, and find the one with the highest maximum
    std::vector<std::pair<TObject*, std::string>> toDraw = { std::make_pair(mc_stack.get(), ""), std::make_pair(h_data.get(), data_drawing_options) };
    for (File& signal: signal_files) {
      toDraw.push_back(std::make_pair(signal.object, getPlotStyle(signal)->drawing_options));
    }

    // Remove NULL items
    toDraw.erase(
        std::remove_if(toDraw.begin(), toDraw.end(), [](std::pair<TObject*, std::string>& element) {
          return element.first == nullptr;
        }), toDraw.end()
      );

    if (!toDraw.size()) {
      std::cerr << "Error: nothing to draw." << std::endl;
      return false;
    };

    // Sort object by minimum
    std::sort(toDraw.begin(), toDraw.end(), [](std::pair<TObject*, std::string> a, std::pair<TObject*, std::string> b) {
        return getMinimum(a.first) < getMinimum(b.first);
      });

    float minimum = getMinimum(toDraw[0].first);

    // Sort objects by maximum
    std::sort(toDraw.begin(), toDraw.end(), [](std::pair<TObject*, std::string> a, std::pair<TObject*, std::string> b) {
        return getMaximum(a.first) > getMaximum(b.first);
      });

    float maximum = getMaximum(toDraw[0].first);

    if ((!h_data.get() || !mc_histo_stat_only.get()))
      plot.show_ratio = false;

    std::shared_ptr<TPad> hi_pad;
    std::shared_ptr<TPad> low_pad;
    if (plot.show_ratio) {
      hi_pad = std::make_shared<TPad>("pad_hi", "", 0., 0.33, 0.99, 0.99);
      hi_pad->Draw();
      hi_pad->SetLeftMargin(0.17);
      hi_pad->SetBottomMargin(0.015);
      hi_pad->SetRightMargin(0.03);

      low_pad = std::make_shared<TPad>("pad_lo", "", 0., 0., 0.99, 0.33);
      low_pad->Draw();
      low_pad->SetLeftMargin(0.17);
      low_pad->SetTopMargin(1.);
      low_pad->SetBottomMargin(0.3);
      low_pad->SetRightMargin(0.03);
      low_pad->SetTickx(1);

      hi_pad->cd();
      if (plot.log_y)
        hi_pad->SetLogy();
    }

    toDraw[0].first->Draw(toDraw[0].second.c_str());
    setRange(toDraw[0].first, plot);

    float safe_margin = 1.20;
    if (plot.log_y)
      safe_margin = 8;

    if (plot.y_axis_range.size() != 2) {
      setMaximum(toDraw[0].first, maximum * safe_margin);

      if (minimum <= 0 && plot.log_y) {
        minimum = 0.1;
      }
      setMinimum(toDraw[0].first, minimum * 1.20);
    }

    // Set x and y axis titles
    setAxisTitles(toDraw[0].first, plot);

    // First, draw MC
    if (mc_stack.get()) {
      mc_stack->Draw("same");
      m_temporaryObjects.push_back(mc_stack);
    }

    // Then, if requested, errors
    if (mc_histo_stat_syst.get() && plot.show_errors) {
      mc_histo_stat_syst->SetMarkerSize(0);
      mc_histo_stat_syst->SetMarkerStyle(0);
      mc_histo_stat_syst->SetFillStyle(m_config.error_fill_style);
      mc_histo_stat_syst->SetFillColor(m_config.error_fill_color);

      mc_histo_stat_syst->Draw("E2 same");
      m_temporaryObjects.push_back(mc_histo_stat_syst);
    }

    // Then signal
    for (File& signal: signal_files) {
      std::string options = getPlotStyle(signal)->drawing_options + " X0 same";
      signal.object->Draw(options.c_str());
    }

    // And finally data
    if (h_data.get()) {
      data_drawing_options += " X0 same";
      h_data->Draw(data_drawing_options.c_str());
      m_temporaryObjects.push_back(h_data);
    }

    // Redraw only axis
    toDraw[0].first->Draw("axis same");

    if (plot.show_ratio) {
      // Compute ratio and draw it
      low_pad->cd();
      low_pad->SetGridy();

      std::shared_ptr<TH1> h_data_cloned(static_cast<TH1*>(h_data->Clone()));
      h_data_cloned->SetDirectory(nullptr);
      h_data_cloned->Divide(mc_histo_stat_only.get());
      h_data_cloned->SetMaximum(2);
      h_data_cloned->SetMinimum(0);
      h_data_cloned->SetLineColor(m_config.error_fill_color);

      h_data_cloned->GetYaxis()->SetTitle("Data / MC");
      h_data_cloned->GetXaxis()->SetTitleOffset(1.10);
      h_data_cloned->GetYaxis()->SetTitleOffset(0.55);
      h_data_cloned->GetXaxis()->SetTickLength(0.06);
      h_data_cloned->GetXaxis()->SetLabelSize(0.085);
      h_data_cloned->GetYaxis()->SetLabelSize(0.07);
      h_data_cloned->GetXaxis()->SetTitleSize(0.09);
      h_data_cloned->GetYaxis()->SetTitleSize(0.08);
      h_data_cloned->GetYaxis()->SetNdivisions(505, true);

      // Compute systematic errors in %
      std::shared_ptr<TH1> h_systematics(static_cast<TH1*>(h_data_cloned->Clone()));
      h_systematics->SetDirectory(nullptr);
      h_systematics->Reset(); // Keep binning
      h_systematics->SetMarkerSize(0);

      for (uint32_t i = 1; i <= (uint32_t) h_systematics->GetNbinsX(); i++) {

        if (mc_histo_syst_only->GetBinContent(i) == 0)
          continue;

        float syst = mc_histo_syst_only->GetBinContent(i) / (mc_histo_syst_only->GetBinError(i) + mc_histo_syst_only->GetBinContent(i));
        h_systematics->SetBinContent(i, 1);
        h_systematics->SetBinError(i, 1 - syst);
      }

      h_systematics->SetFillStyle(m_config.error_fill_style);
      h_systematics->SetFillColor(m_config.error_fill_color);
      h_systematics->Draw("E2");

      h_data_cloned->Draw("P X0 same");

      if (plot.fit_ratio) {
        float xMin = h_data_cloned->GetXaxis()->GetBinLowEdge(1);
        float xMax = h_data_cloned->GetXaxis()->GetBinUpEdge(h_data_cloned->GetXaxis()->GetLast());
        std::shared_ptr<TF1> fct = std::make_shared<TF1>("fit_function", plot.fit_function.c_str(), xMin, xMax);

        h_data_cloned->Fit(fct.get(), "MRNEQ");

        std::shared_ptr<TH1> errors = std::make_shared<TH1D>("errors", "errors", 100, xMin, xMax);
        errors->SetDirectory(nullptr);
        (TVirtualFitter::GetFitter())->GetConfidenceIntervals(errors.get(), 0.68);
        errors->SetStats(false);
        errors->SetMarkerSize(0);
        errors->SetFillColor(m_config.ratio_fit_error_fill_color);
        errors->SetFillStyle(m_config.ratio_fit_error_fill_style);
        errors->Draw("e3 same");

        fct->SetLineWidth(m_config.ratio_fit_line_width);
        fct->SetLineColor(m_config.ratio_fit_line_color);
        fct->SetLineStyle(m_config.ratio_fit_line_style);
        fct->Draw("same");

        if (plot.fit_legend.length() > 0) {
          using namespace boost::io;
          uint32_t fit_parameters = fct->GetNpar();
          boost::format formatter(plot.fit_legend);
          formatter.exceptions( all_error_bits ^ ( too_many_args_bit | too_few_args_bit )  );

          for (uint32_t i = 0; i < fit_parameters; i++) {
            formatter % fct->GetParameter(i);
          }

          std::string legend = formatter.str();

          std::shared_ptr<TLatex> t(new TLatex(plot.fit_legend_position.x, plot.fit_legend_position.y, legend.c_str()));
          t->SetNDC(true);
          t->SetTextFont(42);
          t->SetTextSize(0.07);
          t->Draw();

          m_temporaryObjects.push_back(t);
        }

        m_temporaryObjects.push_back(errors);
        m_temporaryObjects.push_back(fct);
      }

      h_data_cloned->Draw("P X0 same");

      m_temporaryObjects.push_back(h_data_cloned);
      m_temporaryObjects.push_back(h_systematics);
      m_temporaryObjects.push_back(hi_pad);
      m_temporaryObjects.push_back(low_pad);
    }

    gPad->Modified();
    gPad->Update();
    gPad->RedrawAxis();

    if (hi_pad.get())
      hi_pad->cd();

    return true;
  }

  void plotIt::plotAll() {
    // First, explode plots to match all glob patterns

    //expandFiles();
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

      // Load systematics histograms
      for (Systematic& syst: file.systematics) {

        syst.object = nullptr;

        std::shared_ptr<TFile> input_syst(TFile::Open(syst.path.c_str()));
        if (! input_syst.get())
          continue;

        obj = input_syst->Get(plot.name.c_str());
        if (obj) {
          m_temporaryObjects.push_back(input_syst);
          syst.object = obj;
        }
      }

      return true;
    }

    // Should not be possible!
    std::cout << "Error: object '" << plot.name << "' inheriting from '" << plot.inherits_from << "' not found in file '" << file.path << "'" << std::endl;
    return false;
  }

  bool plotIt::expandFiles() {
    std::vector<File> files;

    for (File& file: m_files) {
      std::vector<std::string> matchedFiles = glob(file.path);
      for (std::string& matchedFile: matchedFiles) {
        File f = file;
        f.path = matchedFile;
        //std::cout << file.path << " matches to " << f.path << std::endl;

        files.push_back(f);
      }
    }

    m_files = files;

    return true;
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

  std::shared_ptr<PlotStyle> plotIt::getPlotStyle(File& file) {
    if (file.group.length() && m_groups.count(file.group)) {
      return m_groups[file.group].plot_style;
    } else {
      return file.plot_style;
    }
  }

  void PlotStyle::loadFromYAML(YAML::Node& node, const File& file, plotIt& pIt) {
    if (node["legend"])
      legend = node["legend"].as<std::string>();

    if (file.type == MC)
      legend_style = "lf";
    else if (file.type == SIGNAL)
      legend_style = "l";
    else if (file.type == DATA)
      legend_style = "p";

    if (node["legend-style"])
      legend_style = node["legend-style"].as<std::string>();

    if (node["drawing-options"])
      drawing_options = node["drawing-options"].as<std::string>();
    else {
      if (file.type == MC || file.type == SIGNAL)
        drawing_options = "hist";
      else if (file.type == DATA)
        drawing_options = "P";
    }

    marker_size = -1;
    marker_color = -1;
    marker_type = -1;

    fill_color = -1;
    fill_type = -1;

    line_color = -1;
    line_type = -1;

    if (file.type == MC) {
      fill_color = 1;
      fill_type = 1001;
      line_width = 0;
    } else if (file.type == SIGNAL) {
      fill_type = 0;
      line_color = 1;
      line_width = 1;
      line_type = 2;
    } else {
      marker_size = 1;
      marker_color = 1;
      marker_type = 20;
      line_color = 1;
    }

    if (node["fill-color"])
      fill_color = pIt.loadColor(node["fill-color"]);

    if (node["fill-type"])
      fill_type = node["fill-type"].as<int16_t>();

    if (node["line-color"])
      line_color = pIt.loadColor(node["line-color"]);

    if (node["line-type"])
      line_type = node["line-type"].as<int16_t>();

    if (node["line-width"])
      line_width = node["line-width"].as<float>();

    if (node["marker-color"])
      marker_color = pIt.loadColor(node["marker-color"]);

    if (node["marker-type"])
      marker_type = node["marker-type"].as<int16_t>();

    if (node["marker-size"])
      marker_size = node["marker-size"].as<float>();
  }
}

int main(int argc, char** argv) {

  try {

    TCLAP::CmdLine cmd("Plot histograms", ' ', "0.1");

    TCLAP::ValueArg<std::string> outputFolderArg("o", "output-folder", "output folder", true, "", "string", cmd);

    //TCLAP::SwitchArg dataArg("", "data", "Is this data?", false);
    //TCLAP::SwitchArg mcArg("", "mc", "Is this mc?", false);

    //cmd.xorAdd(dataArg, mcArg);

    //TCLAP::SwitchArg semimuArg("", "semimu", "Is this semi-mu channel?", false);
    //TCLAP::SwitchArg semieArg("", "semie", "Is this semi-e channel?", false);

    //cmd.xorAdd(semimuArg, semieArg);

    //TCLAP::ValueArg<int> btagArg("", "b-tag", "Number of b-tagged jet to require", true, 2, "int", cmd);

    //TCLAP::ValueArg<float> lumiArg("", "lumi", "Luminosity", false, 19700, "int", cmd);

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
    //p.setLuminosity(lumiArg.getValue());

    p.plotAll();

  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }

  return 0;
}
