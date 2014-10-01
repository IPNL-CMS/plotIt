#include <TH1Plotter.h>

#include <TCanvas.h>
#include <TF1.h>
#include <TLatex.h>
#include <TObject.h>
#include <TVirtualFitter.h>

#include <boost/format.hpp>
#include <utilities.h>

namespace plotIt {

  bool TH1Plotter::supports(TObject& object) {
    return object.InheritsFrom("TH1");
  }

  bool TH1Plotter::plot(TCanvas& c, Plot& plot) {
    c.cd();

    // Rescale and style histograms
    for (File& file: m_plotIt.getFiles()) {
      setHistogramStyle(file);

      TH1* h = dynamic_cast<TH1*>(file.object);

      if (file.type != DATA) {
        float factor = m_plotIt.getConfiguration().scale * m_plotIt.getConfiguration().luminosity * file.scale * file.cross_section * file.branching_ratio / file.generated_events;

        float n_entries = h->Integral();
        file.summary.efficiency = n_entries / file.generated_events;
        file.summary.efficiency_error = sqrt( (file.summary.efficiency * (1 - file.summary.efficiency)) / file.generated_events );

        file.summary.n_events = n_entries * factor;
        file.summary.n_events_error = m_plotIt.getConfiguration().scale * m_plotIt.getConfiguration().luminosity * file.cross_section * file.branching_ratio * file.summary.efficiency_error;

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
    std::shared_ptr<THStack> mc_stack;
    std::shared_ptr<TH1> mc_histo_stat_only;
    std::shared_ptr<TH1> mc_histo_syst_only;
    std::shared_ptr<TH1> mc_histo_stat_syst;

    std::shared_ptr<TH1> h_data;
    std::string data_drawing_options;

    std::vector<File> signal_files;

    for (File& file: m_plotIt.getFiles()) {
      if (file.type == MC) {

        TH1* nominal = dynamic_cast<TH1*>(file.object);

        if (mc_stack.get() == nullptr)
          mc_stack = std::make_shared<THStack>("mc_stack", "mc_stack");

        mc_stack->Add(nominal, m_plotIt.getPlotStyle(file)->drawing_options.c_str());
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
          data_drawing_options += m_plotIt.getPlotStyle(file)->drawing_options;
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
      for (File& file: m_plotIt.getFiles()) {
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
      if (m_plotIt.getConfiguration().luminosity_error_percent > 0) {
        // Loop over all bins, and add lumi error
        for (uint32_t i = 1; i <= (uint32_t) mc_histo_syst_only->GetNbinsX(); i++) {
          float error = mc_histo_syst_only->GetBinError(i);
          float entries = mc_histo_syst_only->GetBinContent(i);
          float lumi_error = entries * m_plotIt.getConfiguration().luminosity_error_percent;

          mc_histo_syst_only->SetBinError(i, std::sqrt(error * error + lumi_error * lumi_error));
        }
      }

      // Check if systematic histogram are attached, and add them to the plot
      for (File& file: m_plotIt.getFiles()) {
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
      toDraw.push_back(std::make_pair(signal.object, m_plotIt.getPlotStyle(signal)->drawing_options));
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
      hi_pad->SetLeftMargin(LEFT_MARGIN);
      hi_pad->SetBottomMargin(0.015);
      hi_pad->SetRightMargin(RIGHT_MARGIN);

      low_pad = std::make_shared<TPad>("pad_lo", "", 0., 0., 0.99, 0.33);
      low_pad->Draw();
      low_pad->SetLeftMargin(LEFT_MARGIN);
      low_pad->SetTopMargin(1.);
      low_pad->SetBottomMargin(0.3);
      low_pad->SetRightMargin(RIGHT_MARGIN);
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

    // First, draw MC
    if (mc_stack.get()) {
      mc_stack->Draw("same");
      m_plotIt.addTemporaryObject(mc_stack);
    }

    // Then, if requested, errors
    if (mc_histo_stat_syst.get() && plot.show_errors) {
      mc_histo_stat_syst->SetMarkerSize(0);
      mc_histo_stat_syst->SetMarkerStyle(0);
      mc_histo_stat_syst->SetFillStyle(m_plotIt.getConfiguration().error_fill_style);
      mc_histo_stat_syst->SetFillColor(m_plotIt.getConfiguration().error_fill_color);

      mc_histo_stat_syst->Draw("E2 same");
      m_plotIt.addTemporaryObject(mc_histo_stat_syst);
    }

    // Then signal
    for (File& signal: signal_files) {
      std::string options = m_plotIt.getPlotStyle(signal)->drawing_options + " same";
      signal.object->Draw(options.c_str());
    }

    // And finally data
    if (h_data.get()) {
      data_drawing_options += " E X0 same";
      h_data->Draw(data_drawing_options.c_str());
      m_plotIt.addTemporaryObject(h_data);
    }
    
    // Set x and y axis titles, and default style
    for (auto& obj: toDraw) {
      setDefaultStyle(obj.first);
      setAxisTitles(obj.first, plot);
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

      setDefaultStyle(h_data_cloned.get());
      h_data_cloned->GetYaxis()->SetTickLength(0.04);
      h_data_cloned->GetYaxis()->SetNdivisions(505, true);
      h_data_cloned->GetXaxis()->SetTickLength(0.07);

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

      h_systematics->SetFillStyle(m_plotIt.getConfiguration().error_fill_style);
      h_systematics->SetFillColor(m_plotIt.getConfiguration().error_fill_color);
      h_systematics->Draw("E2");

      h_data_cloned->Draw("P E X0 same");

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
        errors->SetFillColor(m_plotIt.getConfiguration().ratio_fit_error_fill_color);
        errors->SetFillStyle(m_plotIt.getConfiguration().ratio_fit_error_fill_style);
        errors->Draw("e3 same");

        fct->SetLineWidth(m_plotIt.getConfiguration().ratio_fit_line_width);
        fct->SetLineColor(m_plotIt.getConfiguration().ratio_fit_line_color);
        fct->SetLineStyle(m_plotIt.getConfiguration().ratio_fit_line_style);
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
          t->SetTextFont(43);
          t->SetTextSize(LABEL_FONTSIZE - 4);
          t->Draw();

          m_plotIt.addTemporaryObject(t);
        }

        m_plotIt.addTemporaryObject(errors);
        m_plotIt.addTemporaryObject(fct);
      }

      h_data_cloned->Draw("P E X0 same");

      m_plotIt.addTemporaryObject(h_data_cloned);
      m_plotIt.addTemporaryObject(h_systematics);
      m_plotIt.addTemporaryObject(hi_pad);
      m_plotIt.addTemporaryObject(low_pad);
    }

    gPad->Modified();
    gPad->Update();
    gPad->RedrawAxis();

    if (hi_pad.get())
      hi_pad->cd();

    return true;
  
    return true;
  }

  void TH1Plotter::setHistogramStyle(const File& file) {
    TH1* h = dynamic_cast<TH1*>(file.object);

    std::shared_ptr<PlotStyle> style = m_plotIt.getPlotStyle(file);

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

}
