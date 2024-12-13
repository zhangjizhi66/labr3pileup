// main.cc --- 
// 

#include "DataAnalysis.hh"
#include "../UserDefine.hh"

#include "TChain.h"
#include "TTree.h"
#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const char gVERSION[] = "Version: DataAnalysis - 2023.01.16";

int main(int argc, char* argv[])
{
  std::cout<<gVERSION<<std::endl;

  if(argc != 2){
      std::cout << "eg. " << argv[0] << "  [run number]" <<std::endl;
      return 1;
    }

  TString RunNumber(argv[1]);
  int run = RunNumber.Atoi();


  TChain *ipt = new TChain("tree");
#ifdef WAVEFORM
  ipt->Add(TString::Format("%s%s_%04d_W%d_wave.root", FILEPATH, EVENTFILENAME, run, EVENTTIMEWINDOWSWIDTH).Data());
  ipt->Add(TString::Format("%s%s_%04d_W%d_wave_*.root", FILEPATH, EVENTFILENAME, run, EVENTTIMEWINDOWSWIDTH).Data());
#else
  ipt->Add(TString::Format("%s%s_%04d_W%d_notwave.root",FILEPATH, EVENTFILENAME, run, EVENTTIMEWINDOWSWIDTH).Data());
#endif  


  auto opf = new TFile(TString::Format("%s%s_%04d_W%d.root", FILEPATH, CHECKFILENAME, run, EVENTTIMEWINDOWSWIDTH).Data(), "RECREATE");
  auto opt = new TTree("tree", "tree");

  
  auto it = new DataAnalysis(ipt, opf);
  it->Loop(opt);

  //opt->Write();
  opf->Write();
  opf->Close();
    
  std::cout<<'\n'<<"Run "<<run<<" done!"<<'\n'<<'\n';
  
  return 0;
}

// 
// main.cc ends here
