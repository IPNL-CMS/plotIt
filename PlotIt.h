#ifndef PlotIt_h
#define PlotIt_h

#include <sstream>
#include <iomanip>

#include <TROOT.h>
#include <TFile.h>
#include "TH2.h"
#include "THStack.h"
#include "TF1.h"

#include <vector>

struct MyInput
{
  TString fileName;
  TFile*  tfile;

  TString type;
  TString subtype;

  Int_t   ngen;
  Float_t xsec;
  Float_t kfactor;

  Float_t Nexp;
  Float_t ErrNexp;

  Float_t Eff;
  Float_t ErrEff;

};

class PlotIt
{
private:
  TString _inputFile;
  Float_t _lumi;
  Float_t _lumi2a;
  Float_t _lumi2b;
  Float_t _efficiency;
  std::vector<MyInput> _inputs;

public:

  TH1F*   _hdata;
  TH1F*   _hmc;
  TH1F*   _hsig;
  TH1F*   _hqcd;
  TH1F*   _hqcdup;
  TH1F*   _hqcddo;

  THStack* _hstack;
  TH1F*    _htt;
  TH1F*    _hznn;
  TH1F*    _hwln;
  TH1F*    _hdib;
  TH1F*    _hsto;
  TH1F*    _hzll;

  Float_t Ntotdata;
  Float_t Ntotmc;  Float_t ErrNtotmc;
  Float_t Ntotsig; Float_t ErrNtotsig;

  Float_t _ymin;
  Float_t _ymax;

 ~PlotIt();
  PlotIt(const TString& inputFile, const Float_t& efficiency);

  std::string GetLumiLabel() const {
    std::stringstream ss;
    if (_lumi < 1000) {
      ss << std::setprecision(3) << _lumi << " pb^{-1}";
    } else {
      ss << std::setprecision(3) << _lumi / 1000 << " fb^{-1}";
    }

    return ss.str();
  }

  void SetIntputFile(const TString& inputFile) {_inputFile = inputFile;}
  void SetLumi(const Float_t& lumi) { _lumi = lumi;}
  void SetInefficiency(const Float_t& efficiency) { _efficiency = efficiency;}
  void SetYmin(const Float_t& ymin) { _ymin = ymin; }
  void SetYmax(const Float_t& ymax) { _ymax = ymax; }
  void ResetYminmax() { _ymin = -1111; _ymax = -1111; }

  bool init();
  void plotbase(const TString& histo);
  void plot (const TString& histo,Int_t rebin=1,TString xtitle="dummy",TString legpos="right");
  void plot2(const TString& histo,Int_t rebin=1,TString xtitle="dummy",TString legpos="right");
    //  void GetBinContent(const TString& histo,const Int_t& ibin,bool print=true);
  void plotstack(const TString& histo,Int_t rebin=1,Int_t mode=1,TString xtitle="dummy",TString legpos="right");
  void plotstack_ratio(const TString& histo, Int_t rebin=1, Int_t mode=1, TString xtitle="dummy", TString legpos="right");

  void GetBinContent(const TString& histo,const Int_t& ibin,bool print=true);
  TF1* plotqcd(int functype,float lo, float hi,int rebin=1,float upperl=40.,float fixpar = 0.);

  void plotratio();

  std::vector<MyInput> GetInputs() { return _inputs; }

};

#endif // #ifndef PlotIt_h
