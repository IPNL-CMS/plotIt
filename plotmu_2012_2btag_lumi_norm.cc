{
  gROOT->SetBatch();
  gSystem->Load("PlotIt_cc");

  PlotIt p("semimu2012_2btag.list", 0.8393627);

  p.plotstack_ratio("hNGoodJets");
  c1.SaveAs("plots/2btag/nJets_semimu.pdf");

  p.plotstack_ratio("hNBtaggedJets", 1, 1, "Number of b-tagged jets");
  c1.SaveAs("plots/2btag/nBJets_semimu.pdf");

  p.plotstack_ratio("hMuRelIso", 1, 1, "Muon relative isolation");
  c1.SaveAs("plots/2btag/Muon_relative_iso.pdf");

  p.plotstack_ratio("hmttSelected_btag_sel", 5);
  c1.SaveAs("plots/2btag/mtt_2btag_semimu.pdf");

  p.plotstack_ratio("hmttSelected_btag_sel_mass_cut", 5);
  c1.SaveAs("plots/2btag/mtt_2btag_mass_cut_semimu.pdf");

  p.plotstack_ratio("hLeptonPt", 4);
  c1.SaveAs("plots/2btag/Muon_pt.pdf");

  p.plotstack_ratio("h1stjetpt", 4);
  c1.SaveAs("plots/2btag/firstjet_semimu.pdf");

  p.plotstack_ratio("h2ndjetpt", 4);
  c1.SaveAs("plots/2btag/secondjet_semimu.pdf");

  p.plotstack_ratio("h3rdjetpt", 4);
  c1.SaveAs("plots/2btag/thirdjet_semimu.pdf");

  p.plotstack_ratio("h4thjetpt", 4, 1, "4^{th} jet p_{T} [GeV/c]");
  c1.SaveAs("plots/2btag/fourthjet_semimu.pdf");

  p.plotstack_ratio("hMET", 4);
  c1.SaveAs("plots/2btag/MET_semimu.pdf");

  p.plotstack("hNVtx", 2);
  c1.SaveAs("plots/2btag/nvertex_semimu.pdf");

  p.plotstack_ratio("hNVtx", 2);
  c1.SaveAs("plots/2btag/nvertex_ratio_semimu.pdf");

  exit(0);
}
