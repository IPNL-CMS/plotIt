{
  gROOT->SetBatch();
  gSystem->Load("PlotIt_cc");

  PlotIt p("semie2012_2btag.list", 1);

  p.plotstack_ratio("hNGoodJets");
  c1.SaveAs("plots/2btag/nJets_semie.pdf");

  p.plotstack_ratio("hNBtaggedJets", 1, false, 1, "Number of b-tagged jets");
  c1.SaveAs("plots/2btag/nBJets_semie.pdf");

  p.plotstack_ratio("hElRelIso", 1, false, 1, "Electron relative isolation");
  c1.SaveAs("plots/2btag/Electron_relative_iso.pdf");

  p.plotstack_ratio("hmttSelected_btag_sel", 5);
  c1.SaveAs("plots/2btag/mtt_2btag_semie.pdf");

  p.plotstack_ratio("hmttSelected_btag_sel_mass_cut", 5);
  c1.SaveAs("plots/2btag/mtt_2btag_mass_cut_semie.pdf");

  p.plotstack_ratio("hLeptonPt", 4);
  c1.SaveAs("plots/2btag/Electron_pt.pdf");

  p.plotstack_ratio("h1stjetpt", 4);
  c1.SaveAs("plots/2btag/firstjet_semie.pdf");

  p.plotstack_ratio("h2ndjetpt", 4);
  c1.SaveAs("plots/2btag/secondjet_semie.pdf");

  p.plotstack_ratio("h3rdjetpt", 4);
  c1.SaveAs("plots/2btag/thirdjet_semie.pdf");

  p.plotstack_ratio("h4thjetpt", 4, false, 1, "4^{th} jet p_{T} [GeV/c]");
  c1.SaveAs("plots/2btag/fourthjet_semie.pdf");

  p.plotstack_ratio("hMET", 4);
  c1.SaveAs("plots/2btag/MET_semie.pdf");

  p.plotstack_ratio("hNVtx", 2);
  c1.SaveAs("plots/2btag/nvertex_semie.pdf");

  p.plotstack_ratio("hNVtx", 2);
  c1.SaveAs("plots/2btag/nvertex_ratio_semie.pdf");

  exit(0);
}
