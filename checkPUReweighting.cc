#include <TFile.h>
#include <PlotIt.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TROOT.h>

void checkPUReweighting() {
  gROOT->SetBatch(true);
  gSystem->Load("PlotIt_cc");

  PlotIt* e = new PlotIt("semie2012.list", 1);
  e->plotstack("hNTrueInt_nosel", 1);

  TH1* hmc_e = e->_hmc;

  PlotIt* mu = new PlotIt("semimu2012.list", 1);
  mu->plotstack("hNTrueInt_nosel", 1);

  TH1* hmc_mu = mu->_hmc;

  TFile* data_e = TFile::Open("../PUReweighting/Electron_Run2012AB_pileup_profile.root");
  TH1* pu_data_e = static_cast<TH1*>(data_e->Get("pileup"));
  pu_data_e->SetDirectory(NULL);
  
  hmc_e->GetXaxis()->SetTitle("semi-e");
  hmc_e->GetXaxis()->SetTitleSize(0.038);
  hmc_e->SetLineColor(kBlue + 2);
  hmc_e->SetLineWidth(1);

  pu_data_e->GetXaxis()->SetTitle("semi-e");

  TFile* data_mu = TFile::Open("../PUReweighting/Muon_Run2012AB_pileup_profile.root");
  TH1* pu_data_mu = static_cast<TH1*>(data_mu->Get("pileup"));
  pu_data_mu->SetDirectory(NULL);

  hmc_mu->GetXaxis()->SetTitle("semi-mu");
  hmc_mu->GetXaxis()->SetTitleSize(0.038);
  hmc_mu->SetLineColor(kBlue + 2);
  hmc_mu->SetLineWidth(1);

  pu_data_mu->GetXaxis()->SetTitle("semi-mu");

  // Plot
  
  TCanvas* c = new TCanvas("c", "c", 1200, 600);
  c->Divide(2, 1);
  c->cd(1);

  pu_data_e->GetXaxis()->SetLabelSize(); 
  pu_data_mu->GetXaxis()->SetLabelSize(); 

  hmc_e->DrawNormalized("");

  pu_data_e->SetMarkerStyle(20);
  pu_data_e->SetMarkerSize(0.8);
  pu_data_e->DrawNormalized("P same");

  c->cd(2);

  hmc_mu->DrawNormalized("");

  pu_data_mu->SetMarkerStyle(20);
  pu_data_mu->SetMarkerSize(0.8);
  pu_data_mu->DrawNormalized("P same");

  c->Print("plots/PUReweighting_crosscheck.pdf");

  delete e;
  delete mu;
}
