// main.cc --- 
// 

#include "../UserDefine.hh"

#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"
#include "TGraph.h"
#include "TF1.h"
#include "TH1.h"
#include "TCut.h"
#include "TSpectrum.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <vector>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double par[ndet][nid][2];

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "eg. " << argv[0] << "  [run number]" <<std::endl;
        return 1;
    }

    TString RunNumber(argv[1]);
    int run = RunNumber.Atoi();
    
    TFile *opf = new TFile(TString::Format("%s%s_%04d_W%d.root", FILEPATH, CALFILENAME, run, EVENTTIMEWINDOWSWIDTH).Data(), "RECREATE");
    
    TGraph *gr[ndet][nid];
    for (int idet = 0; idet < ndet; idet++)
        for (int id = 0; id < nid; id++){
            if (!valid(idet, id)) continue;
            gr[idet][id] = new TGraph;
        }
    
    TH1D *h[ndet][nid];

    TFile *ipf = new TFile(TString::Format("%s%s_%04d_W%d.root", FILEPATH, ANAFILENAME, run, EVENTTIMEWINDOWSWIDTH).Data());
    TTree *tree = (TTree*)ipf->Get("tree");
    for (int idet = 0; idet < ndet; idet++)
        for (int id = 0; id < nid; id++){
            if (!valid(idet, id)) continue;
                
            tree->Draw(Form("det.ewave>>h%02d%02d(%d,%f,%f)",idet,id,nfitbins,minfitrange,maxfitrange), Form("det.det==%d && det.id==%d && det.twave>%d",idet,id,flushlowedge));
            h[idet][id] = (TH1D*)gROOT->FindObject(Form("h%02d%02d",idet,id));

            TF1 *ffit;
            for (int ipeak = 0; ipeak < int(ve.size()); ipeak++){
                ffit = new TF1("ffit","gaus(0)+pol1(3)",minfitrange,maxfitrange);
                ffit->SetParameter(0, 1000);
                ffit->SetParameter(1, (vchleft[ipeak]+vchright[ipeak])/2.);
                ffit->SetParameter(2, 30);
                ffit->SetParameter(3, 100);
                ffit->SetParameter(4, 0);
                ffit->SetLineColor(kBlack);
                h[idet][id]->Fit(ffit, "QR+", "", vchleft[ipeak], vchright[ipeak]);

                ffit->FixParameter(3, ffit->GetParameter(3));
                ffit->FixParameter(4, ffit->GetParameter(4));
                ffit->SetLineColor(kRed);
                h[idet][id]->Fit(ffit, "QR+", "", ffit->GetParameter(1)-2*ffit->GetParameter(2), ffit->GetParameter(1)+1*ffit->GetParameter(2));

                gr[idet][id]->SetPoint(gr[idet][id]->GetN(), ffit->GetParameter(1), ve[ipeak]);
            }
            opf->cd();
            h[idet][id]->Write();
        }
    ipf->Close();

    
    for (int idet = 0; idet < ndet; idet++)
        for (int id = 0; id < nid; id++){
            if (!valid(idet, id)) continue;
            
            gr[idet][id]->SetName(Form("grfit_det%d_id%d",idet,id));
            gr[idet][id]->Fit("pol1", "Q");
            gr[idet][id]->Write();

            TF1 *f1 = gr[idet][id]->GetFunction("pol1");
            par[idet][id][0] = f1->GetParameter(0);
            par[idet][id][1] = f1->GetParameter(1);
        }
    
    opf->Close();
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  
    std::ofstream ofs_par(Form("%secalpar_%04d.txt", FILEPATH, run), std::ios::out);   // fit parameter output
    
    // fit parameter output
    ofs_par<<std::setprecision(parameterprecision);
    for (int idet = 0; idet < ndet; idet++)
        for (int id = 0; id < nid; id++){
            if (!valid(idet, id)) continue;
            for (int ipar = 0; ipar < 2; ipar++)
                ofs_par<<par[idet][id][ipar]<<'\n';
        }
  
    return 0;
}

// 
// main.cc ends here
