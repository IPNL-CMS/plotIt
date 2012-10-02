{
  gROOT->SetBatch();
  gSystem->Load("PlotIt_cc");

  PlotIt p("semie2012.list", 1);

//  p.plotstack("hNGoodJets_beforesel");
//  c1.SaveAs("plots/nJets_beforesel_semie.pdf");

  p.plotstack("hNGoodJets");
  c1.SaveAs("plots/nJets_semie.pdf");

//  p.plotstack("hNBtaggedJets_beforesel");
//  c1.SaveAs("plots/nBJets_beforesel_semie.pdf");

  p.plotstack("hNBtaggedJets");
  c1.SaveAs("plots/nBJets_semie.pdf");

  p.plotstack("hmttSelected2b", 4);
  c1.SaveAs("plots/mtt_2btag_semie.pdf");

//  p.plotstack("hLeptonPt_beforesel", 4);
//  c1.SaveAs("plots/ElectronPt_beforesel_semie.pdf");

  p.plotstack("hLeptonPt", 4);
  c1.SaveAs("plots/MuonPt_semie.pdf");

  p.plotstack("h1stjetpt", 4);
  c1.SaveAs("plots/firstjet_semie.pdf");

//  p.plotstack("h1stjetpt_beforesel", 4);
//  c1.SaveAs("plots/firstjet_beforesel_semie.pdf");

  p.plotstack("h2ndjetpt", 4);
  c1.SaveAs("plots/secondjet_semie.pdf");

//  p.plotstack("h2ndjetpt_beforesel", 4);
//  c1.SaveAs("plots/secondjet_beforesel_semie.pdf");

  p.plotstack("h3rdjetpt", 4);
  c1.SaveAs("plots/thirdjet_semie.pdf");

//  p.plotstack("h3rdjetpt_beforesel", 4);
//  c1.SaveAs("plots/thirdjet_beforesel_semie.pdf");

  p.plotstack("h4thjetpt", 4);
  c1.SaveAs("plots/fourthjet_semie.pdf");

//  p.plotstack("h4thjetpt_beforesel", 4);
//  c1.SaveAs("plots/fourthjet_beforesel_semie.pdf");

  p.plotstack("hMET", 4);
  c1.SaveAs("plots/MET_semie.pdf");

//  p.plotstack("hMET_beforesel", 4);
//  c1.SaveAs("plots/MET_beforesel_semie.pdf");

  p.plotstack("hNVtx", 2);
  c1.SaveAs("plots/nvertex_semie.pdf");

//  p.plotstack("hNVtx_beforesel", 2);
//  c1.SaveAs("plots/nvertex_beforesel_semie.pdf");

  exit(0);
}
