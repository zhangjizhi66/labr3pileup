// event.cc --- 
// 

#include "DataAnalysis.hh"
#include "../UserDefine.hh"

#include "TMath.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TDirectoryFile.h"
#include "TROOT.h"
#include "TH2.h"
#include "TF1.h"
#include "TFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include <vector>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DataAnalysis::Init()
{
    if (ipt == NULL) return;
    ipt->SetBranchAddress("event", &br_event);
    
    for (int idet = 0; idet < ndet; idet++)
        for (int id = 0; id < nid; id++){
            if ( !valid(idet,id) ) continue;
            
            gROOT->Macro(Form("%scut_%d_%d.C",FILEPATH,idet,id));
            cutg[idet][id] = (TCutG*)gROOT->FindObject(Form("cut_%d_%d",idet,id));
            
            h2fwhm[idet][id] = (TH2D*)ipf_check->Get(Form("h2fwhm/h2fwhm_det%d_id%d",idet,id));
            h2fwhm[idet][id]->ProjectionX(Form("tpjx_det%d_id%d",idet,id));
            TH1F *tpjx = (TH1F*)gROOT->FindObject(Form("tpjx_det%d_id%d",idet,id));
            TH1F *hgrx = (TH1F*)tpjx->Clone("hgrx");
            hgrx->Reset();
            
            grfwhm[idet][id] = (TGraph*)ipf_check->Get(Form("grfwhm_det%d_id%d",idet,id));
            for (int ipnt = 0; ipnt < nfitpnt; ipnt++){
                grpnt_raw[idet][id][ipnt] = (TGraph*)ipf_check->Get(Form("grpnt_det%d_id%d_ipnt%d",idet,id,ipnt));
                grpnt[idet][id][ipnt] = new TGraph;
                grpnt[idet][id][ipnt]->SetName(Form("grpnt_det%d_id%d_ipnt%d",idet,id,ipnt));
            }
            for (int ipnt = 0; ipnt < grfwhm[idet][id]->GetN(); ipnt++)
                if ( cutg[idet][id]->IsInside(grfwhm[idet][id]->GetPointX(ipnt), grfwhm[idet][id]->GetPointY(ipnt)) ){
                    double x = grfwhm[idet][id]->GetPointX(ipnt);
                    
                    int xbin = hgrx->FindBin(x);
                    int xbin0 = hgrx->FindBin(refbin);
                    
                    if ( hgrx->GetBinContent(xbin) >= std::max(tpjx->GetBinContent(xbin0), 1.) )
                        continue;
                    
                    hgrx->Fill(x);
                    
                    for (int jpnt = 0; jpnt < nfitpnt; jpnt++){
                        grpnt[idet][id][jpnt]->SetPoint(grpnt[idet][id][jpnt]->GetN(), grpnt_raw[idet][id][jpnt]->GetPointX(ipnt), grpnt_raw[idet][id][jpnt]->GetPointY(ipnt));
                    }
                }
            
            for (int ipnt = 0; ipnt < nfitpnt; ipnt++){
                fpntpol2[idet][id][ipnt] = new TF1(Form("fpntpol2_det%d_id%d_ipnt%d",idet,id,ipnt), "pol2", 0, maxoverflow);
                grpnt[idet][id][ipnt]->Fit(fpntpol2[idet][id][ipnt], "Q ROB", "", minheight, maxheight);
                grpnt[idet][id][ipnt]->Write();
            }
        }
}

void DataAnalysis::Loop(TTree *opt_)
{
    if (opt_ == NULL) return;

    opt = opt_;
    BranchOpt();

    // TH1/TH2 write
    
    for (int idet = 0; idet < ndet; idet++)
        for (int id = 0; id < nid; id++){
            if ( !valid(idet,id) ) continue;
            
            fepol2[idet][id] = new TF1(Form("fepol2_det%d_id%d",idet,id), "pol2", 0, maxoverflow);
            for (int ipnt = 0; ipnt < nfitpnt; ipnt++){
                fpntpol2[idet][id][ipnt]->Write();
                if ( ipnt - length < rangeuseleft || ipnt - length > rangeuseright ) continue;
                fepol2[idet][id]->SetParameter(0, fepol2[idet][id]->GetParameter(0)+fpntpol2[idet][id][ipnt]->GetParameter(0));
                fepol2[idet][id]->SetParameter(1, fepol2[idet][id]->GetParameter(1)+fpntpol2[idet][id][ipnt]->GetParameter(1));
                fepol2[idet][id]->SetParameter(2, fepol2[idet][id]->GetParameter(2)+fpntpol2[idet][id][ipnt]->GetParameter(2));
            }
            fepol2[idet][id]->Write();
        }
}

void DataAnalysis::BranchOpt()
{
    br_event = 0;

    opt->Branch("det", &vdet);
}

// 
// event.cc ends here
