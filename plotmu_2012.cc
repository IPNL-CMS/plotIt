{
  gROOT->SetBatch();
  gSystem->Load("PlotIt_cc");

  PlotIt p("semimu2012.list", 0.8743);

  p.plotstack_ratio("hNGoodJets");
  c1.SaveAs("plots/nJets_semimu.pdf");

  p.plotstack_ratio("hNBtaggedJets", 1, 1, "Number of b-tagged jets");
  c1.SaveAs("plots/nBJets_semimu.pdf");

  p.plotstack_ratio("hmttSelected2b", 5);
  c1.SaveAs("plots/mtt_2btag_semimu.pdf");

  p.plotstack_ratio("hLeptonPt", 4);
  c1.SaveAs("plots/MuonPt_semimu.pdf");

  p.plotstack_ratio("h1stjetpt", 4);
  c1.SaveAs("plots/firstjet_semimu.pdf");

  p.plotstack_ratio("h2ndjetpt", 4);
  c1.SaveAs("plots/secondjet_semimu.pdf");

  p.plotstack_ratio("h3rdjetpt", 4);
  c1.SaveAs("plots/thirdjet_semimu.pdf");

  p.plotstack_ratio("h4thjetpt", 4, 1, "4^{th} jet p_{T} [GeV/c]");
  c1.SaveAs("plots/fourthjet_semimu.pdf");

  p.plotstack_ratio("hMET", 4);
  c1.SaveAs("plots/MET_semimu.pdf");

  p.plotstack("hNVtx", 2);
  c1.SaveAs("plots/nvertex_semimu.pdf");

  p.plotstack_ratio("hNVtx", 2);
  c1.SaveAs("plots/nvertex_ratio_semimu.pdf");

  exit(0);
}
