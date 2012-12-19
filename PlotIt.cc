//#define PlotIt_cc

#include "PlotIt.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>

#include "TStyle.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TPaveStats.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TMath.h"

#include "TText.h"
#include "TLatex.h"
#include <TROOT.h>
#include <TBrowser.h>

#include "TVirtualFitter.h"
#include "TMinuit.h"

using std::cout;
using std::endl;
using std::vector;

using namespace std;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Double_t fitqcd(Double_t* x, Double_t* par)
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
{
  Double_t val = par[0] * TMath::Power(x[0],par[1]);
  return val;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
double expo_er(double x, double * par, double *matrix)
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
{
  const int npar = 2;

  double derivate[npar]  =
  {
    exp(par[0]+par[1]*x),
    x*exp(par[0]+par[1]*x)
  } ;

  float er=0.;
  for(int i=0; i<npar;i++)
  {
    for(int j=0; j<npar;j++)
    {
      er =er+derivate[i]*( *(matrix+npar*i+j) )*derivate[j];
    }
  }
  // cout<<er<<endl;
  return sqrt(er);

};

//==========================================================================================================

/* d02004style.C

   This routine defines a set of functions to setup

   general, canvas and histogram styles

   recommended by D0. "zmumu_1.C" macro illustrates its usage.

Created: January 2003, Avto Kharchilava
Updated: January 2004,      --"--
*/

// ... Global attributes go here ...
void global_style() {

  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetPadColor(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetFrameBorderMode(0);

  gStyle->SetTitleColor(1);   // 0
  gStyle->SetTitleBorderSize(1);
  gStyle->SetTitleX(0.10);
  gStyle->SetTitleY(0.94);
  gStyle->SetTitleW(0.5);
  gStyle->SetTitleH(0.06);

  gStyle->SetLabelOffset(1e-04);
  gStyle->SetLabelSize(0.2);

  gStyle->SetStatColor(0);
  gStyle->SetStatBorderSize(1);
  gStyle->SetStatX(0.90);
  gStyle->SetStatY(0.90);
  gStyle->SetStatW(0.30);
  gStyle->SetStatH(0.10);

  gStyle->SetErrorX(0.0);   // Horizontal error bar size
  //   gStyle->SetPaperSize(10.,12.);   // Printout size
}

// ... Canvas attributes ...
void canvas_style(TCanvas *c,
    float left_margin=0.15,
    float right_margin=0.05,
    float top_margin=0.10,
    float bottom_margin=0.15,
    int canvas_color=0,
    int frame_color=0) {

  c->SetLeftMargin(left_margin);
  c->SetRightMargin(right_margin);
  c->SetTopMargin(top_margin);
  c->SetBottomMargin(bottom_margin);
  c->SetFillColor(canvas_color);
  c->SetFrameFillColor(frame_color);

  c->SetBorderMode(0);
  c->SetBorderSize(1);
  c->SetFrameBorderMode(0);
}

// ... 1D histogram attributes; (2D to come) ...
void h1_style(TH1 *h,
    int line_width=3,
    int line_color=1,
    int line_style=1, 
    int fill_style=1001,
    int fill_color=50,
    float y_min=-1111.,
    float y_max=-1111.,
    int ndivx=510,
    int ndivy=510,
    int marker_style=20,
    int marker_color=1,
    float marker_size=1.2,
    int optstat=0) {

  h->SetLineWidth(line_width);
  h->SetLineColor(line_color);
  h->SetLineStyle(line_style);
  h->SetFillColor(fill_color);
  h->SetFillStyle(fill_style);
  h->SetMaximum(y_max);
  h->SetMinimum(y_min);
  h->GetXaxis()->SetNdivisions(ndivx);
  h->GetYaxis()->SetNdivisions(ndivy);

  h->SetMarkerStyle(marker_style);
  h->SetMarkerColor(marker_color);
  h->SetMarkerSize(marker_size);
  h->SetStats(optstat);

  h->SetLabelFont(42,"X");       // 42
  h->SetLabelFont(42,"Y");       // 42
  h->SetLabelOffset(0.01,"X");  // D=0.005
  h->SetLabelOffset(0.01,"Y");  // D=0.005
  h->SetLabelSize(0.045,"X");
  h->SetLabelSize(0.045,"Y");
  h->SetTitleOffset(1.0,"X");
  h->SetTitleOffset(1.2,"Y");
  h->SetTitleSize(0.06,"X");
  h->SetTitleSize(0.06,"Y");
  h->SetTitle(0);
  h->SetTitleFont(42, "XYZ");
}

//==============================================================================
PlotIt::~PlotIt()
  //==============================================================================
{
  cout << "PlotIt::~PlotIt() : Finished" << endl;
}

//==============================================================================
PlotIt::PlotIt(const TString& inputFile, const Float_t& efficiency)
  //==============================================================================
{
  cout << "PlotIt::PlotIt" << endl;

  // Initialize private variable :
  _inputFile = inputFile;
  _lumi = 0;
  _efficiency = efficiency;

  _hdata = 0;
  _hmc   = 0;
  _hsig  = 0;
  _hqcd  = 0;
  _hqcddo = 0;
  _hqcdup = 0;

  _hstack = 0;
  _htt  = 0;
  _hznn = 0;
  _hwln = 0;
  _hdib = 0;
  _hsto = 0;
  _hzll = 0;

  // Call the init function which read the input file
  TH1::SetDefaultSumw2(true);

  init();

}

//==============================================================================
bool PlotIt::init()
  //==============================================================================
{
  cout << "PlotIt::init()" << endl;

  _hdata = 0;
  _hmc = 0;
  _hsig = 0;

  _ymin = -1111;
  _ymax = -1111;

  // Initialize d0style :
  //_____________________

  gROOT->Reset();
  gROOT->SetStyle("Plain");
  gROOT->ForceStyle();
  global_style();

  // Open input file :
  //__________________

  cout << _inputFile.Data() << endl;

  ifstream inputFile(_inputFile.Data());
  if (inputFile.bad()) {
    cout << "Impossible to open : " << _inputFile << endl;
    return false;
  }

  // Read the input file :
  //______________________

  std::ostream* aStream = &cout;

  MyInput anInput; 
  int counter = 0;
  cout << "######################################################################################################" << endl;
  while (!inputFile.eof()) {
    counter++;    
    TString first;
    inputFile >> first;
    if (first == "end") break;
    anInput.fileName = first;
    inputFile >> anInput.type >> anInput.ngen >> anInput.xsec >> anInput.kfactor;

    anInput.xsec *= anInput.kfactor;
    anInput.tfile = new TFile(anInput.fileName);
    if      ( anInput.fileName.Contains("QCD")    ) anInput.subtype = "qcd";
    else if ( anInput.fileName.Contains("TTbar")  ) anInput.subtype = "top";
    else if ( anInput.fileName.Contains("VVjets") ) anInput.subtype = "dib";
    else if ( anInput.fileName.Contains("Wjets")  ) anInput.subtype = "wje";
    else if ( anInput.fileName.Contains("Zjets")  ) anInput.subtype = "zje";
    else if ( anInput.fileName.Contains("Zinvisible") ) anInput.subtype = "zinv";
    else if ( anInput.fileName.Contains("SingleTop")  ) anInput.subtype = "stop";
    else                                                anInput.subtype = "none";

    aStream->width(12);		*aStream << "Opening : "; 		
    aStream->width(75);		*aStream << anInput.fileName;
    aStream->width(8);		*aStream << anInput.type;
    aStream->width(8);		*aStream << anInput.subtype;
    aStream->width(10);		*aStream << anInput.ngen;
    aStream->width(12);
    aStream->precision(6);	*aStream << anInput.xsec;
    aStream->width(8);	
    aStream->precision(6);	*aStream << anInput.kfactor;
    *aStream << endl;

    // Lumi. For data, store lumi inside xsec (/pb)
    if (anInput.type == "data") {
      _lumi += anInput.xsec;
    }

    _inputs.push_back(anInput);
    if ( counter > 20 )
      break;
  }
  cout << "######################################################################################################" << endl;

  cout << endl;
  cout << "######################################################################################################" << endl;
  cout << "#  Luminosity: " << _lumi << " /pb" << endl;

  if ( _efficiency != 1. ) {
    cout << " Integrated luminosity mutiplied by : " << _efficiency << endl;
    cout << " New Integrated luminosity          = " << _lumi * _efficiency << endl;
  }

  cout << "######################################################################################################" << endl << endl;

  return true;
}

//==============================================================================
void PlotIt::plot(const TString& histo,Int_t rebin, TString xtitle, TString legpos)
  //==============================================================================
{
  plotbase(histo);

  if (rebin != 1) {
    if (_hdata != 0) _hdata->Rebin(rebin);
    if (_hmc   != 0) _hmc  ->Rebin(rebin);
    if (_hsig  != 0) _hsig ->Rebin(rebin);
  }

  if (_hdata != 0) h1_style(_hdata,3,4,1,1001,50,_ymin,_ymax,510,510,20,1,1.,1);
  if (_hmc   != 0) h1_style(_hmc,  2,1,1,1001,20,_ymin,_ymax,510,510,20,1,1.,1);
  if (_hsig  != 0) h1_style(_hsig, 3,2,2,-1111,0,_ymin,_ymax,510,510,5,1,1.,1);

  if (xtitle != "dummy") _hdata->GetXaxis()->SetTitle(xtitle.Data());

  Float_t inter = _hdata->GetXaxis()->GetXmax() - _hdata->GetXaxis()->GetXmin();
  Short_t num   = (Short_t)(inter/(Float_t)_hdata->GetXaxis()->GetNbins());

  TString ytitle = "";//"Events / ";
  //  ytitle += num;

  _hdata->GetYaxis()->SetTitle(ytitle.Data());

  TCanvas *c1 = new TCanvas("c1"," ",700,600);
  canvas_style(c1);

  Stat_t max = _hmc->GetMaximum()*1.20;
  _hmc->SetMaximum(max);
  max = _hdata->GetMaximum()*1.20;
  if ( max > _hmc->GetMaximum() )  _hmc->SetMaximum(max);

  if (_hdata != 0) _hdata->SetStats(kFALSE);
  if (_hmc   != 0) _hmc  ->SetStats(kFALSE);
  if (_hsig  != 0) _hsig ->SetStats(kFALSE);

  _hmc->Draw("hist");
  if (_hsig != 0 ) _hsig->Draw("same");
  _hdata->Draw("esames");

  TLatex *t1 = new TLatex();
  t1->SetTextFont(42);
  t1->SetTextColor(1);   // 4
  t1->SetTextAlign(12);
  t1->SetTextSize(0.05);
  //  Float_t lx = 0.38 * (_hdata->GetXaxis()->GetXmax()-_hdata->GetXaxis()->GetXmin());
  //  Float_t ly = 0.92 * (_hdata->GetYaxis()->GetXmax()-_hdata->GetYaxis()->GetXmin());
  //  t1->DrawLatex(lx,ly,"D\349 Run II - L=1fb^{-1}");
  //  t1->DrawTextNDC(0.35,0.82,"D\\349 Run II Preliminary");

  Double_t x1=0.30; Double_t y1 = 0.86;
  Double_t x2=0.80; Double_t y2 = 0.98;
  if (legpos != "right") {
    x1=0.20; y1 = 0.65;
    x2=0.40; y2 = 0.85;
  }

  TLegend *leg = new TLegend(x1,y1,x2,y2,NULL,"brNDC");
  leg->SetTextSize(0.04);
  leg->SetFillStyle(1);
  leg->SetFillColor(10);
  leg->SetTextFont(42);
  leg->SetTextAlign(32);

  leg->AddEntry(_hdata,"Data","P");
  leg->AddEntry(_hmc  ,"SM back.","f");
  if ( _hsig != 0 ) leg->AddEntry(_hsig ,"Signal","f");

  leg->Draw();

  gPad->Update();
  gPad->RedrawAxis();
  // Stats
  /*
     TPaveStats *stats1 = (TPaveStats*)_hdata->GetListOfFunctions()->FindObject("stats");
     stats1->SetTextColor(1);
     stats1->SetLineColor(1);
     stats1->SetX1NDC(0.20);  stats1->SetY1NDC(0.83);
     stats1->SetX2NDC(0.37);  stats1->SetY2NDC(0.98);

     TPaveStats *stats2 = (TPaveStats*)_hmc->GetListOfFunctions()->FindObject("stats");
     stats2->SetTextColor(1);
     stats2->SetLineColor(20);
     stats2->SetX1NDC(0.73);  stats2->SetY1NDC(0.83);
     stats2->SetX2NDC(0.90);  stats2->SetY2NDC(0.98);
     */
  gPad->Update();
  gPad->RedrawAxis();
  _hdata->Draw("axissame");

}

//==============================================================================
void PlotIt::plot2(const TString& histo,Int_t rebin, TString xtitle, TString legpos)
  //==============================================================================
{
  plotbase(histo);

  if (rebin != 1) {
    if (_hdata != 0) _hdata->Rebin(rebin);
    if (_hmc   != 0) _hmc  ->Rebin(rebin);
  }

  if (_hdata != 0) h1_style(_hdata,3,4,1,1001,50,_ymin,_ymax,510,510,20,1,1.,1);
  if (_hmc   != 0) h1_style(_hmc,  2,1,1,1001,20,_ymin,_ymax,510,510,20,1,1.,1);

  if (xtitle != "dummy") _hdata->GetXaxis()->SetTitle(xtitle.Data());

  Float_t inter = _hdata->GetXaxis()->GetXmax() - _hdata->GetXaxis()->GetXmin();
  Short_t num   = (Short_t)(inter/(Float_t)_hdata->GetXaxis()->GetNbins());

  TString ytitle = "";//"Events/";
  //  ytitle += num;

  _hdata->GetYaxis()->SetTitle(ytitle.Data());

  TCanvas *c1 = new TCanvas("c1"," ",700,600);
  canvas_style(c1);

  Stat_t max = _hmc->GetMaximum()*1.20;
  _hmc->SetMaximum(max);
  max = _hdata->GetMaximum()*1.20;
  if ( max > _hmc->GetMaximum() )  _hmc->SetMaximum(max);

  _hsig = (TH1F*)_hdata->Clone("HSIG");
  _hsig->Add(_hmc,-1.);
  h1_style(_hsig,  2,2,2,-1,20,_ymin,_ymax,510,510,20,1,1.,1);

  if (_hdata != 0) _hdata->SetStats(kFALSE);
  if (_hmc   != 0) _hmc  ->SetStats(kFALSE);
  if (_hsig  != 0) _hsig ->SetStats(kFALSE);

  _hmc->Draw("hist");
  if (_hsig != 0 ) _hsig->Draw("same");
  _hdata->Draw("esames");

  TLatex *t1 = new TLatex();
  t1->SetTextFont(42);
  t1->SetTextColor(1);   // 4
  t1->SetTextAlign(12);
  t1->SetTextSize(0.05);
  //  Float_t lx = 0.38 * (_hdata->GetXaxis()->GetXmax()-_hdata->GetXaxis()->GetXmin());
  //  Float_t ly = 0.92 * (_hdata->GetYaxis()->GetXmax()-_hdata->GetYaxis()->GetXmin());
  //  t1->DrawLatex(lx,ly,"D\349 Run II - L=1fb^{-1}");
  //  t1->DrawTextNDC(0.35,0.82,"D\\349 Run II Preliminary");

  Double_t x1=0.30; Double_t y1 = 0.86;
  Double_t x2=0.80; Double_t y2 = 0.98;
  if (legpos != "right") {
    x1=0.20; y1 = 0.65;
    x2=0.40; y2 = 0.85;
  }

  TLegend *leg = new TLegend(x1,y1,x2,y2,NULL,"brNDC");
  leg->SetTextSize(0.04);
  leg->SetFillStyle(1);
  leg->SetFillColor(10);
  leg->SetTextFont(42);
  leg->SetTextAlign(32);

  leg->AddEntry(_hdata, std::string("Data (" + GetLumiLabel() + ")").c_str(),"P");
  leg->AddEntry(_hmc  ,"SM back.","f");
  if ( _hsig != 0 ) leg->AddEntry(_hsig ,"Signal","f");

  leg->Draw();

  gPad->Update();
  gPad->RedrawAxis();
  // Stats
  /*
     TPaveStats *stats1 = (TPaveStats*)_hdata->GetListOfFunctions()->FindObject("stats");
     stats1->SetTextColor(1);
     stats1->SetLineColor(1);
     stats1->SetX1NDC(0.20);  stats1->SetY1NDC(0.83);
     stats1->SetX2NDC(0.37);  stats1->SetY2NDC(0.98);

     TPaveStats *stats2 = (TPaveStats*)_hmc->GetListOfFunctions()->FindObject("stats");
     stats2->SetTextColor(1);
     stats2->SetLineColor(20);
     stats2->SetX1NDC(0.73);  stats2->SetY1NDC(0.83);
     stats2->SetX2NDC(0.90);  stats2->SetY2NDC(0.98);
     */
  gPad->Update();
  gPad->RedrawAxis();
  _hdata->Draw("axissame");

}

//==============================================================================
void PlotIt::plotbase(const TString& histo)
  //==============================================================================
{
  std::ostream* aStream = &cout;

  // Loop over input file for MC :
  //______________________________

  if (_hdata != 0) _hdata->Delete();
  if (_hmc   != 0) _hmc  ->Delete();
  if (_hsig  != 0) _hsig ->Delete();

  Ntotdata = 0.;
  Ntotmc   = 0.; ErrNtotmc  = 0.;
  Ntotsig  = 0.; ErrNtotsig = 0.;

  int nmc   = 0;
  int ndata = 0;
  int nsig  = 0;

  for (int i=0; i< (int)_inputs.size(); i++) {

    //    cout << _inputs[i].fileName << endl;
    // Process data
    if (_inputs[i].type == "data") {
      TH1F* tempdata = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (ndata == 0) {
        _hdata = (TH1F*)tempdata->Clone("HDATA");
        _hdata->Reset();
      }
      _hdata->Add(tempdata);
      _inputs[i].Nexp    = (Float_t)tempdata->Integral();
      _inputs[i].ErrNexp = 0.;
      Ntotdata += _inputs[i].Nexp;

      aStream->width(75);  *aStream << _inputs[i].fileName;
      aStream->width(10);  *aStream << _inputs[i].Nexp << endl;

      ++ndata;
    }

    // Process  backgrounds :
    if (_inputs[i].type == "mc") {
      TH1F* tempmc = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (nmc == 0) {
        _hmc = (TH1F*)tempmc->Clone("HMC");
        _hmc->Reset();
      }
      Float_t intlumi = _lumi * _efficiency;
      Float_t Factor   = intlumi * _inputs[i].xsec / (Float_t)_inputs[i].ngen;
      Float_t nentries = (Float_t)tempmc->Integral();

      _inputs[i].Nexp    = nentries * Factor;
      _inputs[i].ErrNexp = Factor * Factor * (nentries*(1. - nentries/(Float_t)_inputs[i].ngen));;

      Ntotmc    += _inputs[i].Nexp;
      ErrNtotmc += _inputs[i].ErrNexp;

      _inputs[i].ErrNexp = sqrt(_inputs[i].ErrNexp);

      _inputs[i].Eff    = nentries/(Float_t)_inputs[i].ngen;
      _inputs[i].ErrEff = 0.;
      if ( _inputs[i].Nexp ) _inputs[i].ErrEff = _inputs[i].Eff * _inputs[i].ErrNexp / _inputs[i].Nexp;

      aStream->width(75);  *aStream << _inputs[i].fileName;
      aStream->width(10);  *aStream << _inputs[i].Nexp;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrNexp;
      *aStream << " | ";
      aStream->width(10);  *aStream << _inputs[i].Eff;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrEff << endl;

      _hmc->Add(tempmc,Factor);

      ++nmc;
    }

    // Process Signal :
    if (_inputs[i].type == "sig") {
      TH1F* tempsig = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (nsig == 0) {
        _hsig = (TH1F*)tempsig->Clone("HSIG");
        _hsig->Reset();
      }
      Float_t Factor   = _lumi * _efficiency * _inputs[i].xsec / (Float_t)_inputs[i].ngen;
      Float_t nentries = (Float_t)tempsig->Integral();

      _inputs[i].Nexp    = nentries * Factor;
      _inputs[i].ErrNexp = Factor * Factor * (nentries *(1. - nentries/(Float_t)_inputs[i].ngen));;

      Ntotsig    += _inputs[i].Nexp;
      ErrNtotsig += _inputs[i].ErrNexp;

      _inputs[i].ErrNexp = sqrt(_inputs[i].ErrNexp);

      _inputs[i].Eff    = nentries/(Float_t)_inputs[i].ngen;
      _inputs[i].ErrEff = 0.;
      if ( _inputs[i].Nexp ) _inputs[i].ErrEff = _inputs[i].Eff * _inputs[i].ErrNexp / _inputs[i].Nexp;

      aStream->width(25);  *aStream << _inputs[i].fileName;
      aStream->width(10);  *aStream << _inputs[i].Nexp;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrNexp;
      *aStream << " | ";
      aStream->width(10);  *aStream << _inputs[i].Eff;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrEff << endl;

      _hsig->Add(tempsig,Factor);

      ++nsig;
    }

  } // end loop

  ErrNtotmc = sqrt(ErrNtotmc);
  ErrNtotsig = sqrt(ErrNtotsig);

  cout << "############################################################################" << endl;
  cout << " data   : " << Ntotdata << endl;
  cout << " back.  : " << Ntotmc   << " +/- " << ErrNtotmc  << endl;
  cout << " signal : " << Ntotsig  << " +/- " << ErrNtotsig << endl;
  cout << "############################################################################" << endl;

}

//==============================================================================
void PlotIt::plotstack(const TString& histo,Int_t rebin, Int_t mode, TString xtitle, TString legpos)
  //==============================================================================
{
  std::ostream* aStream = &cout;

  // Loop over input file for MC :
  //______________________________

  if (_hdata  != 0 ) _hdata->Delete();
  if (_hmc    != 0 ) _hmc->Delete();
  if (_hsig   != 0 ) _hsig->Delete();
  if (_hqcd   != 0) _hqcd->Delete();
  if (_hqcdup != 0) _hqcdup->Delete();
  if (_hqcddo != 0) _hqcddo->Delete();

  if ( _htt    != 0 ) _htt->Delete();
  if ( _hznn   != 0 ) _hznn->Delete();
  if ( _hwln   != 0 ) _hwln->Delete();
  if ( _hdib   != 0 ) _hdib->Delete();
  if ( _hsto   != 0 ) _hsto->Delete();
  if ( _hzll   != 0 ) _hzll->Delete();

  if ( _hstack !=0 ) _hstack->Delete();

  Ntotdata = 0.;
  Ntotmc   = 0.; ErrNtotmc  = 0.;
  Ntotsig  = 0.; ErrNtotsig = 0.;

  int nmc   = 0;
  int ndata = 0;
  int nsig  = 0;

  int ntt  = 0;
  int nznn = 0;
  int nwln = 0;
  int ndib = 0;
  int nsto = 0;
  int nzll = 0;
  int nqcd = 0;

  for (int i=0; i< (int)_inputs.size(); i++) {
    //    cout << _inputs[i].fileName << endl;

    // Process data
    if (_inputs[i].type == "data") {
      TH1F* tempdata = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (tempdata) {
        if (ndata == 0) {
          _hdata = (TH1F*)tempdata->Clone("HDATA");
          _hdata->Reset();
        }
        _hdata->Add(tempdata);
        _inputs[i].Nexp    = (Float_t)tempdata->Integral();
        _inputs[i].ErrNexp = 0.;
        Ntotdata += _inputs[i].Nexp;

        aStream->width(75);  *aStream << _inputs[i].fileName;
        aStream->width(10);  *aStream << _inputs[i].Nexp << endl;

        ndata++;
      }
    }

    // Process  backgrounds :
    if (_inputs[i].type == "mc") {
      TH1F* tempmc = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (nmc == 0) {
        _hmc = (TH1F*)tempmc->Clone("HMC");
        _hmc->Reset();
      }
      Float_t intlumi = _lumi * _efficiency;
      Float_t Factor   = intlumi * _inputs[i].xsec / (Float_t)_inputs[i].ngen;
      Float_t nentries = (Float_t)tempmc->Integral();

      _inputs[i].Nexp    = nentries * Factor;
      _inputs[i].ErrNexp = Factor * Factor * (nentries*(1. - nentries/(Float_t)_inputs[i].ngen));;

      Ntotmc    += _inputs[i].Nexp;
      ErrNtotmc += _inputs[i].ErrNexp;

      _inputs[i].ErrNexp = sqrt(_inputs[i].ErrNexp);

      _inputs[i].Eff    = nentries/(Float_t)_inputs[i].ngen;
      _inputs[i].ErrEff = 0.;
      if ( _inputs[i].Nexp ) _inputs[i].ErrEff = _inputs[i].Eff * _inputs[i].ErrNexp / _inputs[i].Nexp;

      aStream->width(75);  *aStream << _inputs[i].fileName;
      aStream->width(10);  *aStream << _inputs[i].Nexp;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrNexp;
      *aStream << " | ";
      aStream->width(10);  *aStream << _inputs[i].Eff;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrEff << endl;

      _hmc->Add(tempmc,Factor);

      if ( _inputs[i].fileName.Contains("TTbar") ) {
        if ( ntt == 0 ) { _htt = (TH1F*)tempmc->Clone("HTT"); _htt->Reset(); }
        _htt->Add(tempmc,Factor);
        ntt++;
      }

      if (   _inputs[i].fileName.Contains("Zinvisible") ) {
        if ( nznn == 0 ) { _hznn = (TH1F*)tempmc->Clone("HZNN"); _hznn->Reset(); }
        _hznn->Add(tempmc,Factor);
        nznn++;
      }

      if ( _inputs[i].fileName.Contains("Wjets") ) {
        if ( nwln == 0 ) { _hwln = (TH1F*)tempmc->Clone("HWLN"); _hwln->Reset(); }
        _hwln->Add(tempmc,Factor);
        nwln++;
      }

      if ( _inputs[i].fileName.Contains("VVjets") ) {
        if ( ndib == 0 ) { _hdib = (TH1F*)tempmc->Clone("HDIB"); _hdib->Reset(); }
        _hdib->Add(tempmc,Factor);
        ndib++;
      }

      if ( _inputs[i].fileName.Contains("SingleTop") ) {
        if ( nsto == 0 ) { _hsto = (TH1F*)tempmc->Clone("HSTO"); _hsto->Reset(); }
        _hsto->Add(tempmc,Factor);
        nsto++;
      }

      if ( _inputs[i].fileName.Contains("Zjets") ) {
        if ( nzll == 0 ) { _hzll = (TH1F*)tempmc->Clone("HZLL"); _hzll->Reset(); }
        _hzll->Add(tempmc,Factor);
        nzll++;
      }

      if ( _inputs[i].fileName.Contains("QCD") ) {
        if ( nqcd == 0 ) { _hqcd = (TH1F*)tempmc->Clone("HQCD"); _hqcd->Reset(); }
        _hqcd->Add(tempmc,Factor);
        nqcd++;
      }

      nmc++;
    }

    // Process Signal :
    if (_inputs[i].type == "sig") {
      TH1F* tempsig = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (nsig == 0) {
        _hsig = (TH1F*)tempsig->Clone("HSIG");
        _hsig->Reset();
      }
      Float_t Factor   = _lumi * _efficiency * _inputs[i].xsec / (Float_t)_inputs[i].ngen;
      Float_t nentries = (Float_t)tempsig->Integral();

      _inputs[i].Nexp    = nentries * Factor;
      _inputs[i].ErrNexp = Factor * Factor * (nentries *(1. - nentries/(Float_t)_inputs[i].ngen));;

      Ntotsig    += _inputs[i].Nexp;
      ErrNtotsig += _inputs[i].ErrNexp;

      _inputs[i].ErrNexp = sqrt(_inputs[i].ErrNexp);

      _inputs[i].Eff    = nentries/(Float_t)_inputs[i].ngen;
      _inputs[i].ErrEff = 0.;
      if ( _inputs[i].Nexp ) _inputs[i].ErrEff = _inputs[i].Eff * _inputs[i].ErrNexp / _inputs[i].Nexp;

      aStream->width(25);  *aStream << _inputs[i].fileName;
      aStream->width(10);  *aStream << _inputs[i].Nexp;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrNexp;
      *aStream << " | ";
      aStream->width(10);  *aStream << _inputs[i].Eff;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrEff << endl;

      _hsig->Add(tempsig,Factor);

      nsig++;
    }

  } // end loop

  ErrNtotmc = sqrt(ErrNtotmc);
  ErrNtotsig = sqrt(ErrNtotsig);

  cout << "############################################################################" << endl;
  cout << " data   : " << Ntotdata << endl;
  cout << " back.  : " << Ntotmc   << " +/- " << ErrNtotmc  << endl;
  cout << " signal : " << Ntotsig  << " +/- " << ErrNtotsig << endl;
  cout << "############################################################################" << endl;
  if ( _htt  != 0 ) cout << " ttbar   = " << _htt->Integral() << endl;
  if ( _hznn != 0 ) cout << " z->nunu = " << _hznn->Integral() << endl;
  if ( _hwln != 0 ) cout << " W->lnu  = " << _hwln->Integral() << endl;
  if ( _hdib != 0 ) cout << " Dibos.  = " << _hdib->Integral() << endl;
  if ( _hsto != 0 ) cout << " S-top   = " << _hsto->Integral() << endl;
  if ( _hzll != 0 ) cout << " Z->ll   = " << _hzll->Integral() << endl;
  if ( _hqcd != 0 ) cout << " QCD     = " << _hqcd->Integral() << endl;
  cout << "############################################################################" << endl;

  // Hstack :
  //_________

  if (rebin != 1) {
    if (_hdata != 0) _hdata->Rebin(rebin);
    if (_hmc   != 0) _hmc  ->Rebin(rebin);
    if (_hsig  != 0) _hsig ->Rebin(rebin);
    if (_htt   != 0) _htt  ->Rebin(rebin);
    if (_hznn  != 0) _hznn ->Rebin(rebin);
    if (_hwln  != 0) _hwln ->Rebin(rebin);
    if (_hdib  != 0) _hdib ->Rebin(rebin);
    if (_hsto  != 0) _hsto ->Rebin(rebin);
    if (_hzll  != 0) _hzll ->Rebin(rebin);
    if (_hqcd  != 0) _hqcd ->Rebin(rebin);

  }

  if (mode != 1 && _hsig != 0 && _hmc != 0) _hsig->Add(_hmc);

  if (_hdata)
    h1_style(_hdata,3,1,1,1001,50,_ymin,_ymax,510,510,20,1,1.,1);//(_hdata,3,4,1,1001,50,_ymin,_ymax,510,510,20,1,1.,1);

  if (_hmc)
    h1_style(_hmc,  2,1,1,-1111,10,_ymin,_ymax,510,510,20,1,1.,1);

  if ( _hsig != 0 ) {
    if ( mode == 1 ) h1_style(_hsig, 3,1,2,-1111,0,_ymin,_ymax,510,510, 5,1,1.,1);
    else             h1_style(_hsig, 2,1,2,3645, 1,_ymin,_ymax,510,510, 5,1,1.,1);
  }

  //   if ( _hwln != 0 ) h1_style(_hwln, 2, 33,1,1001, 3,_ymin,_ymax,510,510,20,1,1.,1);//33
  //   if ( _hznn != 0 ) h1_style(_hznn, 2,  8,1,1001,  8,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _htt  != 0 ) h1_style(_htt,  2,  2,1,1001,  2,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _hdib != 0 ) h1_style(_hdib, 2,  5,1,1001,  5,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _hsto != 0 ) h1_style(_hsto, 2, 28,1,1001, 28,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _hzll != 0 ) h1_style(_hzll, 2,870,1,1001,870,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _hqcd != 0 ) h1_style(_hqcd, 2,801,1,1001,801,_ymin,_ymax,510,510,20,1,1.,1);

  if ( _hwln != 0 ) h1_style(_hwln, 2, 51,1,1001, 51,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _hznn != 0 ) h1_style(_hznn, 2,  8,1,1001,  8,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _htt  != 0 ) h1_style(_htt,  2,  2,1,1001,  2,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _hdib != 0 ) h1_style(_hdib, 2,  5,1,1001,  5,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _hsto != 0 ) h1_style(_hsto, 2, 28,1,1001, 28,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _hzll != 0 ) h1_style(_hzll, 2,86,1,1001,86,_ymin,_ymax,510,510,20,1,1.,1);//96
  if ( _hqcd != 0 ) h1_style(_hqcd, 2,91,1,1001,91,_ymin,_ymax,510,510,20,1,1.,1);


  _hstack = new THStack("hs","test stacked histograms");
  if ( _hsto != 0 ) _hstack->Add(_hsto);
  if ( _hdib != 0 ) _hstack->Add(_hdib);
  if ( _hznn != 0 ) _hstack->Add(_hznn);
  if ( _hqcd != 0 ) _hstack->Add(_hqcd);
  if ( _hzll != 0 ) _hstack->Add(_hzll);
  if ( _hwln != 0 ) _hstack->Add(_hwln);
  if ( _htt  != 0 ) _hstack->Add(_htt);

  if (_hdata && xtitle != "dummy") _hdata->GetXaxis()->SetTitle(xtitle.Data());

  Float_t inter = _hdata ? _hdata->GetXaxis()->GetXmax() - _hdata->GetXaxis()->GetXmin() : 1;
  Short_t num   = _hdata ? (Short_t)(inter/(Float_t)_hdata->GetXaxis()->GetNbins()) : 1;

  TString ytitle = "";//"Events / ";
  //  ytitle += num;

  if (_hdata)
    _hdata->GetYaxis()->SetTitle(ytitle.Data());

  TCanvas *c1 = new TCanvas("c1"," ",700,600);
  canvas_style(c1);

  if ( _ymax < 0. ) {
    Stat_t max1 = _hdata ? _hdata->GetMaximum()*1.20 : 0;
    Stat_t max2 = _hmc->GetMaximum()*1.20;
    Stat_t max3 = 0.;
    if ( _hsig != 0 ) max3 = _hsig->GetMaximum()*1.20;
    Stat_t max = max1;
    if ( max2 > max ) max = max2;
    if ( max3 > max ) max = max3;
    if (_hdata)
      _hdata->SetMaximum(max);
  } else {
    if (_hdata)
      _hdata->SetMaximum(_ymax);
  }
  if ( _ymin > 0. && _hdata ) _hdata->SetMinimum(_ymin);

  if (_hdata)
    _hdata->SetStats(kFALSE);

  _hmc->SetStats(kFALSE);
  if ( _hsig != 0 ) _hsig->SetStats(kFALSE);

  if ( mode == 1 ) {
    if (_hdata) _hdata->Draw("e");
    _hmc->Draw("histsame");
    _hstack->Draw("histsame");
    if ( _hsig != 0 ) _hsig->Draw("histsame");
    if (_hdata) _hdata->Draw("esame");
  } else {
    if (_hdata) _hdata->Draw("e");
    _hmc->Draw("histsame");
    if ( _hsig != 0 ) _hsig->Draw("histsame");
    _hstack->Draw("histsame");
    if (_hdata) _hdata->Draw("esame");
  }

  _hstack->GetHistogram()->SetLabelFont(42,"X");       // 42
  _hstack->GetHistogram()->SetLabelFont(42,"Y");       // 42
  _hstack->GetHistogram()->SetLabelOffset(0.01,"X");  // D=0.005
  _hstack->GetHistogram()->SetLabelOffset(0.01,"Y");  // D=0.005
  _hstack->GetHistogram()->SetLabelSize(0.045,"X");
  _hstack->GetHistogram()->SetLabelSize(0.045,"Y");

  _hstack->GetHistogram()->SetTitleOffset(1.0,"X");
  _hstack->GetHistogram()->SetTitleOffset(1.2,"Y");

  _hstack->GetHistogram()->SetTitleSize(0.06,"X");
  _hstack->GetHistogram()->SetTitleSize(0.06,"Y");
  _hstack->GetHistogram()->SetTitle(0);

  //  TText *t1 = new TText();
  TLatex *t1 = new TLatex();
  t1->SetTextFont(42);
  t1->SetTextColor(1);   // 4
  t1->SetTextAlign(12);
  t1->SetTextSize(0.05);
  //t1->DrawTextNDC(0.38,0.92,"D0 Run II Preliminary");
  //t1->DrawTextNDC(0.38,0.92,"D0 Run II Preliminary");
  //  Float_t lx = 0.38 * (_hdata->GetXaxis()->GetXmax()-_hdata->GetXaxis()->GetXmin());
  //  Float_t ly = 0.92 * (_hdata->GetYaxis()->GetXmax()-_hdata->GetYaxis()->GetXmin());
  //  t1->DrawLatex(lx,ly,"D\\349 Run II - L=310pb^{-1}");
  //  t1->DrawTextNDC(0.35,0.93,"D\\349 Run II Preliminary");

  Double_t x1=0.63; Double_t y1 = 0.63;
  Double_t x2=0.93; Double_t y2 = 0.88;
  if (legpos != "right") {
    x1=0.17; y1 = 0.63;
    x2=0.37; y2 = 0.88;
  }

  TLegend *leg = new TLegend(x1,y1,x2,y2,NULL,"brNDC");
  leg->SetTextSize(0.03);
  leg->SetFillStyle(1);
  leg->SetFillColor(10);
  leg->SetTextFont(42);
  leg->SetTextAlign(32);
  leg->SetBorderSize(1);

  if (_hdata)
    leg->AddEntry(_hdata, std::string("Data (" + GetLumiLabel() + ")").c_str(), "P"); 

  if ( _htt  != 0 )  leg->AddEntry(_htt,"t#bar{t}","f");
  if ( _hwln != 0 )  leg->AddEntry(_hwln,"W#rightarrow l#nu + jets","f");
  if ( _hzll != 0 )  leg->AddEntry(_hzll,"Z#rightarrow l^{+}l^{-}+ jets","f");
  if ( _hqcd != 0 )  leg->AddEntry(_hqcd,"QCD","f");
  if ( _hznn != 0 )  leg->AddEntry(_hznn,"Z#rightarrow #nu#nu + jets","f");
  if ( _hdib != 0 )  leg->AddEntry(_hdib,"WW,WZ,ZZ","f");
  if ( _hsto != 0 )  leg->AddEntry(_hsto,"single-t","f");
  if ( _hsig != 0 )  leg->AddEntry(_hsig,"Signal","f");

  leg->Draw();

  gPad->Update();
  gPad->RedrawAxis();
  // Stats
  /*
     TPaveStats *stats1 = (TPaveStats*)_hdata->GetListOfFunctions()->FindObject("stats");
     stats1->SetTextColor(1);
     stats1->SetLineColor(1);
     stats1->SetX1NDC(0.20);  stats1->SetY1NDC(0.83);
     stats1->SetX2NDC(0.37);  stats1->SetY2NDC(0.98);

     TPaveStats *stats2 = (TPaveStats*)_hmc->GetListOfFunctions()->FindObject("stats");
     stats2->SetTextColor(1);
     stats2->SetLineColor(20);
     stats2->SetX1NDC(0.73);  stats2->SetY1NDC(0.83);
     stats2->SetX2NDC(0.90);  stats2->SetY2NDC(0.98);
     */

  gPad->Update();
  gPad->RedrawAxis();
  //  _hdata->Draw("axissame");

}

//==============================================================================
void PlotIt::plotstack_ratio(const TString& histo,Int_t rebin, Int_t mode, TString xtitle, TString legpos)
  //==============================================================================
{
  std::ostream* aStream = &cout;

  // Loop over input file for MC :
  //______________________________

  if (_hdata  != 0 ) _hdata->Delete();
  if (_hmc    != 0 ) _hmc->Delete();
  if (_hsig   != 0 ) _hsig->Delete();
  if (_hqcd   != 0) _hqcd->Delete();
  if (_hqcdup != 0) _hqcdup->Delete();
  if (_hqcddo != 0) _hqcddo->Delete();

  if ( _htt    != 0 ) _htt->Delete();
  if ( _hznn   != 0 ) _hznn->Delete();
  if ( _hwln   != 0 ) _hwln->Delete();
  if ( _hdib   != 0 ) _hdib->Delete();
  if ( _hsto   != 0 ) _hsto->Delete();
  if ( _hzll   != 0 ) _hzll->Delete();

  if ( _hstack !=0 ) _hstack->Delete();

  Ntotdata = 0.;
  Ntotmc   = 0.; ErrNtotmc  = 0.;
  Ntotsig  = 0.; ErrNtotsig = 0.;

  int nmc   = 0;
  int ndata = 0;
  int nsig  = 0;

  int ntt  = 0;
  int nznn = 0;
  int nwln = 0;
  int ndib = 0;
  int nsto = 0;
  int nzll = 0;
  int nqcd = 0;

  for (int i=0; i< (int)_inputs.size(); i++) {
    //    cout << _inputs[i].fileName << endl;

    // Process data
    if (_inputs[i].type == "data") {
      TH1F* tempdata = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (tempdata) {
        if (ndata == 0) {
          _hdata = (TH1F*)tempdata->Clone("HDATA");
          _hdata->Reset();
        }
        _hdata->Add(tempdata);
        _inputs[i].Nexp    = (Float_t)tempdata->Integral();
        _inputs[i].ErrNexp = 0.;
        Ntotdata += _inputs[i].Nexp;

        aStream->width(75);  *aStream << _inputs[i].fileName;
        aStream->width(10);  *aStream << _inputs[i].Nexp << endl;

        ndata++;
      }
    }

    // Process  backgrounds :
    if (_inputs[i].type == "mc") {
      TH1F* tempmc = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (nmc == 0) {
        _hmc = (TH1F*)tempmc->Clone("HMC");
        _hmc->Reset();
      }
      Float_t intlumi = _lumi * _efficiency;
      Float_t Factor   = intlumi * _inputs[i].xsec / (Float_t)_inputs[i].ngen;
      Float_t nentries = (Float_t)tempmc->Integral();

      _inputs[i].Nexp    = nentries * Factor;
      _inputs[i].ErrNexp = Factor * Factor * (nentries*(1. - nentries/(Float_t)_inputs[i].ngen));;

      Ntotmc    += _inputs[i].Nexp;
      ErrNtotmc += _inputs[i].ErrNexp;

      _inputs[i].ErrNexp = sqrt(_inputs[i].ErrNexp);

      _inputs[i].Eff    = nentries/(Float_t)_inputs[i].ngen;
      _inputs[i].ErrEff = 0.;
      if ( _inputs[i].Nexp ) _inputs[i].ErrEff = _inputs[i].Eff * _inputs[i].ErrNexp / _inputs[i].Nexp;

      aStream->width(75);  *aStream << _inputs[i].fileName;
      aStream->width(10);  *aStream << _inputs[i].Nexp;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrNexp;
      *aStream << " | ";
      aStream->width(10);  *aStream << _inputs[i].Eff;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrEff << endl;

      _hmc->Add(tempmc,Factor);

      if ( _inputs[i].fileName.Contains("TTbar") ) {
        if ( ntt == 0 ) { _htt = (TH1F*)tempmc->Clone("HTT"); _htt->Reset(); }
        _htt->Add(tempmc,Factor);
        ntt++;
      }

      if (   _inputs[i].fileName.Contains("Zinvisible") ) {
        if ( nznn == 0 ) { _hznn = (TH1F*)tempmc->Clone("HZNN"); _hznn->Reset(); }
        _hznn->Add(tempmc,Factor);
        nznn++;
      }

      if ( _inputs[i].fileName.Contains("Wjets") ) {
        if ( nwln == 0 ) { _hwln = (TH1F*)tempmc->Clone("HWLN"); _hwln->Reset(); }
        _hwln->Add(tempmc,Factor);
        nwln++;
      }

      if ( _inputs[i].fileName.Contains("VVjets") ) {
        if ( ndib == 0 ) { _hdib = (TH1F*)tempmc->Clone("HDIB"); _hdib->Reset(); }
        _hdib->Add(tempmc,Factor);
        ndib++;
      }

      if ( _inputs[i].fileName.Contains("SingleTop") ) {
        if ( nsto == 0 ) { _hsto = (TH1F*)tempmc->Clone("HSTO"); _hsto->Reset(); }
        _hsto->Add(tempmc,Factor);
        nsto++;
      }

      if ( _inputs[i].fileName.Contains("Zjets") ) {
        if ( nzll == 0 ) { _hzll = (TH1F*)tempmc->Clone("HZLL"); _hzll->Reset(); }
        _hzll->Add(tempmc,Factor);
        nzll++;
      }

      if ( _inputs[i].fileName.Contains("QCD") ) {
        if ( nqcd == 0 ) { _hqcd = (TH1F*)tempmc->Clone("HQCD"); _hqcd->Reset(); }
        _hqcd->Add(tempmc,Factor);
        nqcd++;
      }

      nmc++;
    }

    // Process Signal :
    if (_inputs[i].type == "sig") {
      TH1F* tempsig = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (nsig == 0) {
        _hsig = (TH1F*)tempsig->Clone("HSIG");
        _hsig->Reset();
      }
      Float_t Factor   = _lumi * _efficiency * _inputs[i].xsec / (Float_t)_inputs[i].ngen;
      Float_t nentries = (Float_t)tempsig->Integral();

      _inputs[i].Nexp    = nentries * Factor;
      _inputs[i].ErrNexp = Factor * Factor * (nentries *(1. - nentries/(Float_t)_inputs[i].ngen));;

      Ntotsig    += _inputs[i].Nexp;
      ErrNtotsig += _inputs[i].ErrNexp;

      _inputs[i].ErrNexp = sqrt(_inputs[i].ErrNexp);

      _inputs[i].Eff    = nentries/(Float_t)_inputs[i].ngen;
      _inputs[i].ErrEff = 0.;
      if ( _inputs[i].Nexp ) _inputs[i].ErrEff = _inputs[i].Eff * _inputs[i].ErrNexp / _inputs[i].Nexp;

      aStream->width(25);  *aStream << _inputs[i].fileName;
      aStream->width(10);  *aStream << _inputs[i].Nexp;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrNexp;
      *aStream << " | ";
      aStream->width(10);  *aStream << _inputs[i].Eff;
      aStream->width(5);   *aStream << " +/- ";
      aStream->width(10);  *aStream << _inputs[i].ErrEff << endl;

      _hsig->Add(tempsig,Factor);

      nsig++;
    }

  } // end loop

  ErrNtotmc = sqrt(ErrNtotmc);
  ErrNtotsig = sqrt(ErrNtotsig);

  cout << "############################################################################" << endl;
  cout << " data   : " << Ntotdata << endl;
  cout << " back.  : " << Ntotmc   << " +/- " << ErrNtotmc  << endl;
  cout << " signal : " << Ntotsig  << " +/- " << ErrNtotsig << endl;
  cout << "############################################################################" << endl;
  if ( _htt  != 0 ) cout << " ttbar   = " << _htt->Integral() << endl;
  if ( _hznn != 0 ) cout << " z->nunu = " << _hznn->Integral() << endl;
  if ( _hwln != 0 ) cout << " W->lnu  = " << _hwln->Integral() << endl;
  if ( _hdib != 0 ) cout << " Dibos.  = " << _hdib->Integral() << endl;
  if ( _hsto != 0 ) cout << " S-top   = " << _hsto->Integral() << endl;
  if ( _hzll != 0 ) cout << " Z->ll   = " << _hzll->Integral() << endl;
  if ( _hqcd != 0 ) cout << " QCD     = " << _hqcd->Integral() << endl;
  cout << "############################################################################" << endl;

  // Hstack :
  //_________

  if (rebin != 1) {
    if (_hdata != 0) _hdata->Rebin(rebin);
    if (_hmc   != 0) _hmc  ->Rebin(rebin);
    if (_hsig  != 0) _hsig ->Rebin(rebin);
    if (_htt   != 0) _htt  ->Rebin(rebin);
    if (_hznn  != 0) _hznn ->Rebin(rebin);
    if (_hwln  != 0) _hwln ->Rebin(rebin);
    if (_hdib  != 0) _hdib ->Rebin(rebin);
    if (_hsto  != 0) _hsto ->Rebin(rebin);
    if (_hzll  != 0) _hzll ->Rebin(rebin);
    if (_hqcd  != 0) _hqcd ->Rebin(rebin);

  }

  if (mode != 1 && _hsig != 0 && _hmc != 0) _hsig->Add(_hmc);

  if (_hdata)
    h1_style(_hdata, 3, 1, 1, 1001, 50, _ymin, _ymax, 510, 510, 20, 1, 1., 1);//(_hdata,3,4,1,1001,50,_ymin,_ymax,510,510,20,1,1.,1);

  if (_hmc)
    h1_style(_hmc,  2,1,1,-1111,10,_ymin,_ymax,510,510,20,1,1.,1);

  if ( _hsig != 0 ) {
    if ( mode == 1 ) h1_style(_hsig, 3,1,2,-1111,0,_ymin,_ymax,510,510, 5,1,1.,1);
    else             h1_style(_hsig, 2,1,2,3645, 1,_ymin,_ymax,510,510, 5,1,1.,1);
  }

  //   if ( _hwln != 0 ) h1_style(_hwln, 2, 33,1,1001, 3,_ymin,_ymax,510,510,20,1,1.,1);//33
  //   if ( _hznn != 0 ) h1_style(_hznn, 2,  8,1,1001,  8,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _htt  != 0 ) h1_style(_htt,  2,  2,1,1001,  2,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _hdib != 0 ) h1_style(_hdib, 2,  5,1,1001,  5,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _hsto != 0 ) h1_style(_hsto, 2, 28,1,1001, 28,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _hzll != 0 ) h1_style(_hzll, 2,870,1,1001,870,_ymin,_ymax,510,510,20,1,1.,1);
  //   if ( _hqcd != 0 ) h1_style(_hqcd, 2,801,1,1001,801,_ymin,_ymax,510,510,20,1,1.,1);

  if ( _hwln != 0 ) h1_style(_hwln, 2, 51,1,1001, 51,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _hznn != 0 ) h1_style(_hznn, 2,  8,1,1001,  8,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _htt  != 0 ) h1_style(_htt,  2,  2,1,1001,  2,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _hdib != 0 ) h1_style(_hdib, 2,  5,1,1001,  5,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _hsto != 0 ) h1_style(_hsto, 2, 28,1,1001, 28,_ymin,_ymax,510,510,20,1,1.,1);
  if ( _hzll != 0 ) h1_style(_hzll, 2,86,1,1001,86,_ymin,_ymax,510,510,20,1,1.,1);//96
  if ( _hqcd != 0 ) h1_style(_hqcd, 2,91,1,1001,91,_ymin,_ymax,510,510,20,1,1.,1);


  _hstack = new THStack("hs","test stacked histograms");
  if ( _hsto != 0 ) _hstack->Add(_hsto);
  if ( _hdib != 0 ) _hstack->Add(_hdib);
  if ( _hznn != 0 ) _hstack->Add(_hznn);
  if ( _hqcd != 0 ) _hstack->Add(_hqcd);
  if ( _hzll != 0 ) _hstack->Add(_hzll);
  if ( _hwln != 0 ) _hstack->Add(_hwln);
  if ( _htt  != 0 ) _hstack->Add(_htt);

  if (_hdata && xtitle != "dummy") _hdata->GetXaxis()->SetTitle(xtitle.Data());

  Float_t inter = _hdata ? _hdata->GetXaxis()->GetXmax() - _hdata->GetXaxis()->GetXmin() : 1;
  float num   = _hdata ? (inter / (Float_t) _hdata->GetXaxis()->GetNbins()) : 1;

  TString ytitle = TString::Format("Events / %.02f", num);

  if (_hdata)
    _hdata->GetYaxis()->SetTitle(ytitle.Data());

  TCanvas *c1 = new TCanvas("c1", " ", 700, 900);
  canvas_style(c1);
  c1->cd();

  // Data / MC comparison
  TPad* pad_hi = new TPad("pad_hi", "", 0., 0.33, 0.99, 0.99);
  pad_hi->Draw();
  pad_hi->SetLeftMargin(0.17);
  pad_hi->SetBottomMargin(0.015);
  pad_hi->SetRightMargin(0.015);

  TPad* pad_lo = new TPad("pad_lo", "", 0., 0., 0.99, 0.33);
  pad_lo->Draw();
  pad_lo->SetLeftMargin(0.17);
  pad_lo->SetTopMargin(1.);
  pad_lo->SetBottomMargin(0.3);
  pad_lo->SetRightMargin(0.015);
  pad_lo->SetTickx(1);

  pad_hi->cd();

  if ( _ymax < 0. ) {
    Stat_t max1 = _hdata ? _hdata->GetMaximum()*1.20 : 0;
    Stat_t max2 = _hmc->GetMaximum()*1.20;
    Stat_t max3 = 0.;
    if ( _hsig != 0 ) max3 = _hsig->GetMaximum()*1.20;
    Stat_t max = max1;
    if ( max2 > max ) max = max2;
    if ( max3 > max ) max = max3;
    if (_hdata)
      _hdata->SetMaximum(max);
  } else {
    if (_hdata)
      _hdata->SetMaximum(_ymax);
  }
  if ( _ymin > 0. && _hdata ) _hdata->SetMinimum(_ymin);

  if (_hdata)
    _hdata->SetStats(kFALSE);

  _hmc->SetStats(kFALSE);
  if ( _hsig != 0 ) _hsig->SetStats(kFALSE);

  if ( mode == 1 ) {
    if (_hdata) _hdata->Draw("e");
    _hmc->Draw("histsame");
    _hstack->Draw("histsame");
    if ( _hsig != 0 ) _hsig->Draw("histsame");
    if (_hdata) _hdata->Draw("esame");
  } else {
    if (_hdata) _hdata->Draw("e");
    _hmc->Draw("histsame");
    if ( _hsig != 0 ) _hsig->Draw("histsame");
    _hstack->Draw("histsame");
    if (_hdata) _hdata->Draw("esame");
  }

  if (_hdata) {
    _hdata->GetYaxis()->SetTitleOffset(1.8);
    _hdata->GetYaxis()->SetTitleSize(0.045);

    _hdata->GetXaxis()->SetLabelSize(0.);

    if (xtitle == "dummy")
      xtitle = _hdata->GetXaxis()->GetTitle();

    _hdata->GetXaxis()->SetTitle(NULL);
    _hdata->GetXaxis()->SetTitleOffset(0);
    _hdata->GetXaxis()->SetTitleSize(0);
  }

  _hstack->GetHistogram()->SetLabelFont(42,"X");       // 42
  _hstack->GetHistogram()->SetLabelFont(42,"Y");       // 42

  _hstack->GetHistogram()->SetLabelOffset(0.007,"X");  // D=0.005
  _hstack->GetHistogram()->SetLabelOffset(0.007,"Y");  // D=0.005

  _hstack->GetHistogram()->SetLabelSize(0,"X");
  _hstack->GetHistogram()->SetLabelSize(0.045,"Y");

  _hstack->GetHistogram()->SetTitleOffset(1.8, "X");
  _hstack->GetHistogram()->SetTitleOffset(0.55, "Y");

  _hstack->GetHistogram()->SetTitleSize(0.09, "X");
  _hstack->GetHistogram()->SetTitleSize(0.09, "Y");

  _hstack->GetHistogram()->SetTitleFont(42, "XY");

  _hstack->GetHistogram()->SetTitle(0);

  //  TText *t1 = new TText();
  TLatex *t1 = new TLatex();
  t1->SetTextFont(42);
  t1->SetTextColor(1);   // 4
  t1->SetTextAlign(12);
  t1->SetTextSize(0.05);
  //t1->DrawTextNDC(0.38,0.92,"D0 Run II Preliminary");
  //t1->DrawTextNDC(0.38,0.92,"D0 Run II Preliminary");
  //  Float_t lx = 0.38 * (_hdata->GetXaxis()->GetXmax()-_hdata->GetXaxis()->GetXmin());
  //  Float_t ly = 0.92 * (_hdata->GetYaxis()->GetXmax()-_hdata->GetYaxis()->GetXmin());
  //  t1->DrawLatex(lx,ly,"D\\349 Run II - L=310pb^{-1}");
  //  t1->DrawTextNDC(0.35,0.93,"D\\349 Run II Preliminary");

  Double_t x1=0.64; Double_t y1 = 0.60;
  Double_t x2=0.94; Double_t y2 = 0.85;
  if (legpos != "right") {
    x1=0.17; y1 = 0.63;
    x2=0.37; y2 = 0.88;
  }

  TLegend *leg = new TLegend(x1,y1,x2,y2,NULL,"brNDC");
  leg->SetTextSize(0.03);
  leg->SetFillStyle(1);
  leg->SetFillColor(10);
  leg->SetTextFont(42);
  leg->SetTextAlign(32);
  leg->SetBorderSize(1);

  if (_hdata)
    leg->AddEntry(_hdata, std::string("Data (" + GetLumiLabel() + ")").c_str(), "P"); 

  if ( _htt  != 0 )  leg->AddEntry(_htt,"t#bar{t}","f");
  if ( _hwln != 0 )  leg->AddEntry(_hwln,"W#rightarrow l#nu + jets","f");
  if ( _hzll != 0 )  leg->AddEntry(_hzll,"Z#rightarrow l^{+}l^{-}+ jets","f");
  if ( _hqcd != 0 )  leg->AddEntry(_hqcd,"QCD","f");
  if ( _hznn != 0 )  leg->AddEntry(_hznn,"Z#rightarrow #nu#nu + jets","f");
  if ( _hdib != 0 )  leg->AddEntry(_hdib,"WW,WZ,ZZ","f");
  if ( _hsto != 0 )  leg->AddEntry(_hsto,"single-t","f");
  if ( _hsig != 0 )  leg->AddEntry(_hsig,"Signal","f");

  leg->Draw();

  gPad->Update();
  gPad->RedrawAxis();
  // Stats
  /*
     TPaveStats *stats1 = (TPaveStats*)_hdata->GetListOfFunctions()->FindObject("stats");
     stats1->SetTextColor(1);
     stats1->SetLineColor(1);
     stats1->SetX1NDC(0.20);  stats1->SetY1NDC(0.83);
     stats1->SetX2NDC(0.37);  stats1->SetY2NDC(0.98);

     TPaveStats *stats2 = (TPaveStats*)_hmc->GetListOfFunctions()->FindObject("stats");
     stats2->SetTextColor(1);
     stats2->SetLineColor(20);
     stats2->SetX1NDC(0.73);  stats2->SetY1NDC(0.83);
     stats2->SetX2NDC(0.90);  stats2->SetY2NDC(0.98);
     */

  gPad->Update();
  gPad->RedrawAxis();
  //  _hdata->Draw("axissame");
  //
  pad_lo->cd();

  pad_lo->SetGridy();

  TH1* data_clone = static_cast<TH1*>(_hdata->Clone("hdata_cloned"));
  data_clone->Divide(_hmc);
  data_clone->SetMaximum(1.5);
  data_clone->SetMinimum(0.5);

  if (xtitle != "dummy")
    data_clone->SetXTitle(xtitle.Data());
  
  data_clone->SetYTitle("Data / MC");
  data_clone->SetXTitle(xtitle.Data());

  data_clone->GetXaxis()->SetTitleOffset(1.10);
  data_clone->GetYaxis()->SetTitleOffset(0.55);
  data_clone->GetXaxis()->SetTickLength(0.06);
  data_clone->GetXaxis()->SetLabelSize(0.085);
  data_clone->GetYaxis()->SetLabelSize(0.07);
  data_clone->GetXaxis()->SetTitleSize(0.09);
  data_clone->GetYaxis()->SetTitleSize(0.08);
  data_clone->GetYaxis()->SetNdivisions(505,true);

  data_clone->Draw("e");

  // Fit the ratio
  TF1* ratioFit = new TF1("ratioFit", "[0]");
  ratioFit->SetParameter(0, 1.);
  ratioFit->SetLineColor(46);
  ratioFit->SetLineWidth(1.5);
  data_clone->Fit(ratioFit, "Q");

  ratioFit->Draw("same");

  double fitValue = ratioFit->GetParameter(0);
  double fitError = ratioFit->GetParError(0);

  TPaveText* fitlabel = new TPaveText(0.65, 0.77, 0.98, 0.83, "brNDC");
  fitlabel->SetTextFont(42);
  fitlabel->SetTextSize(0.08);
  fitlabel->SetFillColor(0);
  TString fitLabelText = TString::Format("Fit: %.4f #pm %.4f", fitValue, fitError);
  fitlabel->AddText(fitLabelText);

  fitlabel->Draw("same");

  gPad->Update();
  gPad->RedrawAxis();
}

//==============================================================================
void PlotIt::GetBinContent(const TString& histo, const Int_t& ibin, bool print)
  //==============================================================================
{
  std::ostream* aStream = &cout;

  // Loop over input file for MC :
  //______________________________

  if (_hdata != 0) _hdata->Delete();
  if (_hmc   != 0) _hmc  ->Delete();
  if (_hsig  != 0) _hsig ->Delete();

  Ntotdata = 0.;
  Ntotmc   = 0.; ErrNtotmc  = 0.;
  Ntotsig  = 0.; ErrNtotsig = 0.;

  int nmc   = 0;
  int ndata = 0;
  int nsig  = 0;

  for (int i=0; i< (int)_inputs.size(); i++) {
    //    cout << _inputs[i].fileName << endl;

    // Process data
    if (_inputs[i].type == "data") {
      TH1F* tempdata = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (ndata == 0) {
        _hdata = (TH1F*)tempdata->Clone("HDATA");
        _hdata->Reset();
      }
      _hdata->Add(tempdata);
      _inputs[i].Nexp    = (Float_t)tempdata->GetBinContent(ibin);
      _inputs[i].ErrNexp = 0.;
      Ntotdata += _inputs[i].Nexp;

      if ( print ) {
        aStream->width(25);  *aStream << _inputs[i].fileName;
        aStream->width(10);  *aStream << _inputs[i].Nexp << endl;
      }
      ndata++;
    }

    // Process  backgrounds :
    if (_inputs[i].type == "mc") {
      TH1F* tempmc = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (nmc == 0) {
        _hmc = (TH1F*)tempmc->Clone("HMC");
        _hmc->Reset();
      }
      Float_t intlumi = _lumi * _efficiency;
      Float_t Factor   = intlumi * _inputs[i].xsec / (Float_t)_inputs[i].ngen;
      Float_t nentries = (Float_t)tempmc->GetBinContent(ibin);

      _inputs[i].Nexp    = nentries * Factor;
      _inputs[i].ErrNexp = Factor * Factor * (nentries*(1. - nentries/(Float_t)_inputs[i].ngen));;

      Ntotmc    += _inputs[i].Nexp;
      ErrNtotmc += _inputs[i].ErrNexp;

      _inputs[i].ErrNexp = sqrt(_inputs[i].ErrNexp);

      if ( print ) {
        aStream->width(25);  *aStream << _inputs[i].fileName;
        aStream->width(10);  *aStream << _inputs[i].Nexp;
        aStream->width(5);   *aStream << " +/- ";
        aStream->width(10);  *aStream << _inputs[i].ErrNexp << endl;
      }

      _hmc->Add(tempmc,Factor);

      nmc++;
    }

    // Process Signal :
    if (_inputs[i].type == "sig") {
      TH1F* tempsig = (TH1F*)_inputs[i].tfile->Get(histo.Data());
      if (nsig == 0) {
        _hsig = (TH1F*)tempsig->Clone("HSIG");
        _hsig->Reset();
      }
      Float_t Factor   = _lumi * _efficiency * _inputs[i].xsec / (Float_t)_inputs[i].ngen;
      Float_t nentries = (Float_t)tempsig->GetBinContent(ibin);

      _inputs[i].Nexp    = nentries * Factor;
      _inputs[i].ErrNexp = Factor * Factor * (nentries *(1. - nentries/(Float_t)_inputs[i].ngen));;

      Ntotsig    += _inputs[i].Nexp;
      ErrNtotsig += _inputs[i].ErrNexp;

      _inputs[i].ErrNexp = sqrt(_inputs[i].ErrNexp);

      _inputs[i].Eff    = nentries/(Float_t)_inputs[i].ngen;
      _inputs[i].ErrEff = 0.;
      if ( _inputs[i].Nexp ) _inputs[i].ErrEff = _inputs[i].Eff * _inputs[i].ErrNexp / _inputs[i].Nexp;

      if ( print ) {
        aStream->width(25);  *aStream << _inputs[i].fileName;
        aStream->width(10);  *aStream << _inputs[i].Nexp;
        aStream->width(5);   *aStream << " +/- ";
        aStream->width(10);  *aStream << _inputs[i].ErrNexp;
        *aStream << " | ";
        aStream->width(10);  *aStream << _inputs[i].Eff;
        aStream->width(5);   *aStream << " +/- ";
        aStream->width(10);  *aStream << _inputs[i].ErrEff << endl;
      }

      _hsig->Add(tempsig,Factor);

      nsig++;
    }

  } // end loop

  ErrNtotmc = sqrt(ErrNtotmc);
  ErrNtotsig = sqrt(ErrNtotsig);

  if ( print ) {
    cout << "############################################################################" << endl;
    cout << " data   : " << Ntotdata << endl;
    cout << " back.  : " << Ntotmc   << " +/- " << ErrNtotmc  << endl;
    cout << " signal : " << Ntotsig  << " +/- " << ErrNtotsig << endl;
    cout << "############################################################################" << endl;
  }
}

//==============================================================================
TF1* PlotIt::plotqcd(int functype,float lo, float hi,int rebin,float upperl, float fixpar)
  //==============================================================================
{
  TF1* func;

  //  cout << "Reset" << endl;
  //  _hdata->Add(_hsig);

  //  cout <<"Clone" << endl;
  TH1F* hdata = (TH1F*)_hdata->Clone("HDATA");
  TH1F* hmc   = (TH1F*)_hmc->Clone("HSIG");

  hdata->SetStats(kFALSE);
  hmc->SetStats(kFALSE);

  //  hdata->Sumw2();
  //  hmc->Sumw2();

  hdata->Add(hmc,-1.);

  //  cout << "Fit" << endl;
  if ( functype == 1 ) {
    func = new TF1("myfunc",fitqcd,lo,500.,2);
  } else {
    func = new TF1("myfunc","expo",lo,500.);
    cout << "Fixed Parameter: " << fixpar << endl;

    if ( fixpar != 0. ) func->FixParameter(1,fixpar);
  }

  hdata->Fit(func,"0F","",lo,hi);

  Double_t par0 = func->GetParameter(0);
  Double_t par1 = func->GetParameter(1);

  double par_expo[2];
  double matrix_expo[2][2];

  for(int i=0;i<2;i++) par_expo[i]= func->GetParameter(i);
  gMinuit->mnemat(&matrix_expo[0][0],2); //matrice variance-covariance

  _hqcd = new TH1F("hqcd","hqcd",_hdata->GetNbinsX(),hdata->GetBinLowEdge(1),hdata->GetBinLowEdge(_hdata->GetNbinsX())+hdata->GetBinWidth(_hdata->GetNbinsX()));

  _hqcdup = new TH1F("hqcdup","hqcdup",_hdata->GetNbinsX(),hdata->GetBinLowEdge(1),hdata->GetBinLowEdge(_hdata->GetNbinsX())+hdata->GetBinWidth(_hdata->GetNbinsX()));

  _hqcddo = new TH1F("hqcddo","hqcddo",_hdata->GetNbinsX(),hdata->GetBinLowEdge(1),hdata->GetBinLowEdge(_hdata->GetNbinsX())+hdata->GetBinWidth(_hdata->GetNbinsX()));

  //  cout << "Loop" << endl;
  bool firstBinFound = false;
  for (int i=1; i<=hdata->GetNbinsX(); ++i) {

    //    cout << "Bin : "  << i << endl;

    if ( !firstBinFound && _hdata->GetBinContent(i) > 0. ) firstBinFound = true;
    if ( !firstBinFound ) continue;

    //    cout << "Pass" << endl;

    //    Float_t binLow = hdata->GetBinLowEdge(i);
    //    Float_t binHig = binLow + hdata->GetBinWidth(i);
    //    Float_t val    = func->Integral(binLow,binHig)/hdata->GetBinWidth(i);

    Float_t val = func->Eval(hdata->GetBinCenter(i));
    Float_t er  = expo_er(hdata->GetBinCenter(i), par_expo, &matrix_expo[0][0]);

    _hqcd->SetBinContent(i,val);
    _hqcd->SetBinError(i,0.);

    _hqcdup->SetBinContent(i,val+er);
    _hqcddo->SetBinContent(i,val-er);

  }

  //===========================================================================
  // Evaluate QCD background:
  //===========================================================================

  Float_t binWidth = _hqcd->GetBinWidth(10);
  Int_t firstBin   = (int)(upperl/binWidth)+1;

  Float_t qcd = _hqcd->Integral(firstBin,500);
  Float_t qcdup = _hqcdup->Integral(firstBin,500);
  Float_t qcddo = _hqcddo->Integral(firstBin,500);

  cout << "==========================================================" << endl;
  cout << " QCD         = " << setw(10) << qcd << endl;
  cout << " QCD (up)    = " << setw(10) << qcdup << endl;
  cout << " QCD (down)  = " << setw(10) << qcddo << endl;
  cout << " QCD (final) = "
    << setw(10) << qcd
    << " +" << setw(10) << abs(qcd-qcdup)
    << " -" << setw(10) << abs(qcd-qcddo)
    << endl;
  cout << "==========================================================" << endl;

  h1_style(_hqcd, 2,1,1,1001,10,0.,1.,510,510,20,1,1.,1);

  _hmc->Add(_hqcd,1.);
  _hstack->Add(_hqcd);

  //  cout << "Rebin" << endl;

  if (rebin != 1) {
    if (_hdata != 0) _hdata->Rebin(rebin);
    if (_hmc   != 0) _hmc->Rebin(rebin);
    if (_hsig  != 0) _hsig->Rebin(rebin);
    if (_hqcd  != 0) _hqcd->Rebin(rebin);

    if (_htt   != 0) _htt->Rebin(rebin);
    if (_hznn  != 0) _hznn->Rebin(rebin);
    if (_hwln  != 0) _hwln->Rebin(rebin);
    if (_hdib  != 0) _hdib->Rebin(rebin);
    if (_hsto  != 0) _hsto->Rebin(rebin);
    if (_hzll  != 0) _hzll->Rebin(rebin);

    h1_style(_hdata,3,4,1,1001,50,_ymin,_ymax,510,510,20,1,1.,1);
    h1_style(_hmc,  2,1,1,-1111,0,_ymin,_ymax,510,510,20,1,1.,1);
    h1_style(_hsig, 3,1,2,-1111,0,_ymin,_ymax,510,510, 5,1,1.,1);

    //    if ( mode == 1 ) h1_style(_hsig, 3,1,2,-1111,0,_ymin,_ymax,510,510, 5,1,1.,1);
    //    else             h1_style(_hsig, 2,1,2,3645, 1,_ymin,_ymax,510,510, 5,1,1.,1);

    if ( _hwln != 0 ) h1_style(_hwln, 2,33,1,1001,33,_ymin,_ymax,510,510,20,1,1.,1);
    if ( _hznn != 0 ) h1_style(_hznn, 2,8,1,1001,8,_ymin,_ymax,510,510,20,1,1.,1);
    if ( _htt  != 0 ) h1_style(_htt,  2,2,1,1001,2,_ymin,_ymax,510,510,20,1,1.,1);
    if ( _hdib != 0 ) h1_style(_hdib, 2,5,1,1001,5,_ymin,_ymax,510,510,20,1,1.,1);
    if ( _hsto != 0 ) h1_style(_hsto, 2,28,1,1001,28,_ymin,_ymax,510,510,20,1,1.,1);
    if ( _hzll != 0 ) h1_style(_hzll, 2,9,1,1001,9,_ymin,_ymax,510,510,20,1,1.,1);

    //    TList* mylist = _hstack->GetHists();
    //    for (int i=0;i<mylist->GetSize();++i) {
    //      TH1F* tmp = (TH1F*)mylist->At(i);
    //      tmp->Rebin(rebin);
    //    }
  }

  //  cout << "Draw" << endl;

  _hdata->SetStats(kFALSE);
  _hmc->SetStats(kFALSE);
  _hsig->SetStats(kFALSE);
  _hqcd->SetStats(kFALSE);

  _hdata->Draw("e");
  _hmc->Draw("histsame");
  _hstack->Draw("histsame");
  _hsig->Draw("histsame");
  _hdata->Draw("esame");

  //  func->SetLineColor(3);
  //  func->SetLineWidth(3);
  //  func->SetLineStyle(2);
  //  func->SetRange(lo,500.);
  //  func->Draw("same");

  _hstack->GetHistogram()->SetLabelFont(42,"X");       // 42
  _hstack->GetHistogram()->SetLabelFont(42,"Y");       // 42
  _hstack->GetHistogram()->SetLabelOffset(0.01,"X");  // D=0.005
  _hstack->GetHistogram()->SetLabelOffset(0.01,"Y");  // D=0.005
  _hstack->GetHistogram()->SetLabelSize(0.045,"X");
  _hstack->GetHistogram()->SetLabelSize(0.045,"Y");
  _hstack->GetHistogram()->SetTitleOffset(1.0,"X");
  _hstack->GetHistogram()->SetTitleOffset(1.2,"Y");
  _hstack->GetHistogram()->SetTitleSize(0.06,"X");
  _hstack->GetHistogram()->SetTitleSize(0.06,"Y");
  _hstack->GetHistogram()->SetTitle(0);

  //  TText *t1 = new TText();
  TLatex *t1 = new TLatex();
  t1->SetTextFont(42);
  t1->SetTextColor(1);   // 4
  t1->SetTextAlign(12);
  t1->SetTextSize(0.05);
  //t1->DrawTextNDC(0.38,0.92,"D0 Run II Preliminary");
  //t1->DrawTextNDC(0.38,0.92,"D0 Run II Preliminary");
  Float_t lx = 0.38 * (_hdata->GetXaxis()->GetXmax()-_hdata->GetXaxis()->GetXmin());
  Float_t ly = 0.92 * (_hdata->GetYaxis()->GetXmax()-_hdata->GetYaxis()->GetXmin());
  //  t1->DrawLatex(lx,ly,"D\\349 Run II - L=310pb^{-1}");
  //  t1->DrawTextNDC(0.35,0.93,"D\\349 Run II Preliminary");

  Double_t x1=0.73; Double_t y1 = 0.58;
  Double_t x2=0.93; Double_t y2 = 0.88;

  TLegend *leg = new TLegend(x1,y1,x2,y2,NULL,"brNDC");
  leg->SetTextSize(0.03);
  leg->SetFillStyle(1);
  leg->SetFillColor(10);
  leg->SetTextFont(42);
  leg->SetTextAlign(32);
  leg->SetBorderSize(1);

  leg->AddEntry(_hdata,"Data","P");

  if ( _hqcd != 0 )  leg->AddEntry(_hqcd,"QCD","f");
  if ( _hwln != 0 )  leg->AddEntry(_hwln,"W#rightarrow l#nu + jets","f");
  if ( _hznn != 0 )  leg->AddEntry(_hznn,"Z#rightarrow #nu#nu + jets","f");
  if ( _htt  != 0 )  leg->AddEntry(_htt,"t#bar{t}","f");
  if ( _hdib != 0 )  leg->AddEntry(_hdib,"WW,WZ,ZZ","f");
  if ( _hzll != 0 )  leg->AddEntry(_hzll,"Z#rightarrow l^{+}l^{-}+ jets","f");
  if ( _hsto != 0 )  leg->AddEntry(_hsto,"single-t","f");
  if ( _hsig != 0 )  leg->AddEntry(_hsig,"Signal","f");

  leg->Draw();

  gPad->Update();
  gPad->RedrawAxis();

  //  cout << "Delete" << endl;

  hdata->Delete();
  hmc->Delete();
  //  func->Delete();

  return func;

}

//==============================================================================
void PlotIt::plotratio()
  //==============================================================================
{
  _hdata->Divide(_hmc);
  TCanvas *c2 = new TCanvas("c2"," ",700,600);
  canvas_style(c2);

  _hdata->SetMaximum(2.);
  _hdata->SetMinimum(0.);

  _hdata->Draw("e");

}
