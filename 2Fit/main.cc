// main.cc --- 
// 

#include "DataAnalysis.hh"
#include "../UserDefine.hh"

#include "TChain.h"
#include "TTree.h"
#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include "TRint.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const char gVERSION[] = "Version: DataAnalysis - 2023.01.16";

int main(int argc, char* argv[])
{
  std::cout<<gVERSION<<std::endl;

  //TRint *theApp = new TRint("Rint",&argc,argv);
    
  if(argc < 2)
    {
      std::cout << "eg. " << argv[0] << "  [run number]" <<std::endl;
      return 1;
    }

  TString RunNumber(argv[1]);
  int run = RunNumber.Atoi();
   
  Long64_t startentry = -1, stopentry = -1;
  if (argc >= 4){
    TString startNumber(argv[2]);
    startentry = startNumber.Atoi();

    TString stopNumber(argv[3]);
    stopentry = stopNumber.Atoi();
  }

  TChain *ipt = new TChain("tree");
#ifdef WAVEFORM
  ipt->Add(TString::Format("%s%s_%04d_W%d_wave.root", FILEPATH, EVENTFILENAME, run, EVENTTIMEWINDOWSWIDTH).Data());
  ipt->Add(TString::Format("%s%s_%04d_W%d_wave_*.root", FILEPATH, EVENTFILENAME, run, EVENTTIMEWINDOWSWIDTH).Data());
#else
  ipt->Add(TString::Format("%s%s_%04d_W%d_notwave.root",FILEPATH, EVENTFILENAME, run, EVENTTIMEWINDOWSWIDTH).Data());
#endif  
   
#ifdef USE_CURRENT_STDWAVE
  TFile *ipf_wave = new TFile(Form("%s%s_%04d_W%d.root", FILEPATH, STANDARDWAVEFILENAME, run, EVENTTIMEWINDOWSWIDTH));
#elif defined USE_FIXED_STDWAVE
  TFile *ipf_wave = new TFile(Form("%s%s_%04d_W%d.root", FILEPATH, STANDARDWAVEFILENAME, USE_FIXED_STDWAVE, EVENTTIMEWINDOWSWIDTH));
#endif

  TString opfname = TString::Format("%s%s_%04d_W%d.root", FILEPATH, ANAFILENAME, run, EVENTTIMEWINDOWSWIDTH);
  if (argc >= 4){
      if ( startentry > ipt->GetEntries() && stopentry > ipt->GetEntries() )
          return 0;
      
      opfname = TString::Format("%s%s_%04d_W%d_%lld_%lld.root", FILEPATH, ANAFILENAME, run, EVENTTIMEWINDOWSWIDTH, startentry, stopentry);
  }
    
  auto opf = new TFile(opfname.Data(), "RECREATE");
  auto opt = new TTree("tree", "tree");

  auto it = new DataAnalysis(ipt, ipf_wave);
  it->Loop(opf, opt, startentry, stopentry);

  opt->Write();
  opf->Close();
  ipf_wave->Close();
    
  //theApp->Run();
  //delete theApp;
    
  if (argc >= 4)
      std::cout<<'\n'<<"Run "<<run<<" entry "<<startentry<<"-"<<stopentry<<" done!"<<'\n'<<'\n';
  else
      std::cout<<'\n'<<"Run "<<run<<" done!"<<'\n'<<'\n';
  
  return 0;
}

// 
// main.cc ends here
