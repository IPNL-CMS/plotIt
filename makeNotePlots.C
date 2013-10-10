{
  gSystem->Load("PlotIt_cc");
  PlotIt pmu("semimu2011.list",4678.,0.8049); // cheat

  pmu.plotstack("hmttSelected2b",4);
  c1.SaveAs("mtt_2btag_semimu_4_7_scaled.pdf");
  pmu.plotstack("hLeptonPt", 4);
  c1.SaveAs("MuonPt_semimu_4_7fb.pdf");
  pmu.plotstack("h1stjetpt",4);
  c1.SaveAs("firstjet_semimu_4_7fb.pdf");
  pmu.plotstack("h2ndjetpt",4);
  c1.SaveAs("secondjet_semimu_4_7fb.pdf");
  pmu.plotstack("h3rdjetpt",4);
  c1.SaveAs("thirdjet_semimu_4_7fb.pdf");
  pmu.plotstack("h4thjetpt",4);
  c1.SaveAs("fourthjet_semimu_4_7fb.pdf");
  pmu.plotstack("hMET",4);
  c1.SaveAs("MET_semimu_4_7fb.pdf");



  PlotIt pe("semie2011.list",4682.,0.9124); //cheat -- a changer éventuellement
  pe.plotstack("hmttSelected2b",4);
  c1.SaveAs("mtt_2btag_semie_4_7_scaled.pdf");
  pe.plotstack("hLeptonPt", 4);
  c1.SaveAs("ElectronPt_semie_4_7fb.pdf");
  pe.plotstack("h1stjetpt",4);
  c1.SaveAs("firstjet_semie_4_7fb.pdf");
  pe.plotstack("h2ndjetpt",4);
  c1.SaveAs("secondjet_semie_4_7fb.pdf");
  pe.plotstack("h3rdjetpt",4);
  c1.SaveAs("thirdjet_semie_4_7fb.pdf");
  pe.plotstack("h4thjetpt",4);
  c1.SaveAs("fourthjet_semie_4_7fb.pdf");
  pe.plotstack("hMET",4);
  c1.SaveAs("MET_semie_4_7fb.pdf");


}
