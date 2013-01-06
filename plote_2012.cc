{
  gROOT->SetBatch();
  gSystem->Load("PlotIt_cc");

  PlotIt p("semie2012.list", 0.91);

  p.plotstack_ratio("hNGoodJets");
  c1.SaveAs("plots/nJets_semie.pdf");

  p.plotstack_ratio("hNBtaggedJets", 1, 1, "Number of b-tagged jets");
  c1.SaveAs("plots/nBJets_semie.pdf");

  p.plotstack_ratio("hmttSelected2b", 5);
  c1.SaveAs("plots/mtt_2btag_semie.pdf");

  p.plotstack_ratio("hLeptonPt", 4);
  c1.SaveAs("plots/ElectronPt_semie.pdf");

  p.plotstack_ratio("h1stjetpt", 4);
  c1.SaveAs("plots/firstjet_semie.pdf");

  p.plotstack_ratio("h2ndjetpt", 4);
  c1.SaveAs("plots/secondjet_semie.pdf");

  p.plotstack_ratio("h3rdjetpt", 4);
  c1.SaveAs("plots/thirdjet_semie.pdf");

  p.plotstack_ratio("h4thjetpt", 4, 1, "4^{th} jet p_{T} [GeV/c]");
  c1.SaveAs("plots/fourthjet_semie.pdf");

  p.plotstack_ratio("hMET", 4);
  c1.SaveAs("plots/MET_semie.pdf");

  p.plotstack_ratio("hNVtx", 2);
  c1.SaveAs("plots/nvertex_semie.pdf");

  p.plotstack_ratio("hNVtx", 2);
  c1.SaveAs("plots/nvertex_ratio_semie.pdf");

  exit(0);
}
