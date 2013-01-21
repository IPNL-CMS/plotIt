{
  gROOT->SetBatch();
  gSystem->Load("PlotIt_cc");

  PlotIt p("semimu2012_1btag.list", 0.946654613);

  p.plotstack_ratio("hNGoodJets");
  c1.SaveAs("plots/1btag/nJets_semimu.pdf");

  p.plotstack_ratio("hNBtaggedJets", 1, false, 1, "Number of b-tagged jets");
  c1.SaveAs("plots/1btag/nBJets_semimu.pdf");

  p.plotstack_ratio("hMuRelIso", 1, false, 1, "Muon relative isolation");
  c1.SaveAs("plots/1btag/Muon_relative_iso.pdf");

  p.plotstack_ratio("hmttSelected_btag_sel", 5);
  c1.SaveAs("plots/1btag/mtt_1btag_semimu.pdf");

  p.plotstack_ratio("hmttSelected_btag_sel", 5, true);
  c1.SaveAs("plots/1btag/mtt_1btag_semimu_log.pdf");

  p.plotstack_ratio("hmttSelected_btag_sel_mass_cut", 5);
  c1.SaveAs("plots/1btag/mtt_1btag_mass_cut_semimu.pdf");

  p.plotstack_ratio("hLeptonPt", 4);
  c1.SaveAs("plots/1btag/Muon_pt.pdf");

  p.plotstack_ratio("h1stjetpt", 4);
  c1.SaveAs("plots/1btag/firstjet_semimu.pdf");

  p.plotstack_ratio("h2ndjetpt", 4);
  c1.SaveAs("plots/1btag/secondjet_semimu.pdf");

  p.plotstack_ratio("h3rdjetpt", 4);
  c1.SaveAs("plots/1btag/thirdjet_semimu.pdf");

  p.plotstack_ratio("h4thjetpt", 4, false, 1, "4^{th} jet p_{T} [GeV/c]");
  c1.SaveAs("plots/1btag/fourthjet_semimu.pdf");

  p.plotstack_ratio("hMET", 4);
  c1.SaveAs("plots/1btag/MET_semimu.pdf");

  p.plotstack("hNVtx", 2);
  c1.SaveAs("plots/1btag/nvertex_semimu.pdf");

  p.plotstack_ratio("hNVtx", 2);
  c1.SaveAs("plots/1btag/nvertex_ratio_semimu.pdf");

  exit(0);
}
