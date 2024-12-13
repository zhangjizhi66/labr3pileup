// event.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 一 9月 21 16:28:37 2020 (+0800)
// Last-Updated: 四 4月 27 22:57:22 2023 (+0800)
//	     By: Hongyi Wu(吴鸿毅)
//     Update #: 123
// URL: http://wuhongyi.cn

#include "DataAnalysis.hh"
#include "../UserDefine.hh"

#include "TMath.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TDirectoryFile.h"
#include "TROOT.h"
#include "TF1.h"
#include "TFile.h"
#include "TFitResult.h"
#include "TSpectrum.h"
#include "TMarker.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <map>
#include <vector>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DataAnalysis::Init()
{
    if (ipt == NULL) return;
    ipt->SetBranchAddress("det", &br_vdet);
    
    std::ifstream ifs(Form("%secalpar_%04d.txt",FILEPATH,run));  // fit parameter input
    for (int idet = 0; idet < ndet; idet++)
        for (int id = 0; id < nid; id++){
            if (!valid(idet, id)) continue;
            for (int ipar = 0; ipar < 2; ipar++)
                ifs>>par[idet][id][ipar];
        }
    
    hdt = new TH1D("hdt","",tbins,tmin,tmax);
    h2xy = new TH2D("hg2xy","",ebins,emin,emax,ebins,emin,emax);
}

void DataAnalysis::Loop(TFile *opf_, TTree *opt_, Long64_t startentry = -1, Long64_t stopentry = -1)
{
    if (opt_ == NULL) return;
    
    opf_->cd();

    opt = opt_;
    BranchOpt();
    
    clock_t start = clock(), stop = clock();
    
    if (startentry == -1 && stopentry == -1){
        startentry = 0;
        stopentry = ipt->GetEntries();
    }
    
    if ( stopentry > ipt->GetEntries() )
        stopentry = ipt->GetEntries();
  
    for (Long64_t jentry = startentry; jentry < stopentry; jentry++){
        ipt->GetEntry(jentry);
        //if (jentry>1000) break;
        
        vdet.clear();
        vdet = *br_vdet;
        
        for (int ihit = 0; ihit < int(vdet.size()); ihit++){
            vdet[ihit].wave.clear();
            if (!valid(vdet[ihit].det, vdet[ihit].id)) continue;
            
            for (int ipeak = 0; ipeak < int(vdet[ihit].ewave.size()); ipeak++)
                vdet[ihit].ewave[ipeak] = par[vdet[ihit].det][vdet[ihit].id][0] + par[vdet[ihit].det][vdet[ihit].id][1] * vdet[ihit].ewave[ipeak];
        }
        
        for (int ihit = 0; ihit < int(vdet.size()); ihit++){
            if (!valid(vdet[ihit].det, vdet[ihit].id)) continue;
            for (int ipeak = 0; ipeak < int(vdet[ihit].twave.size()); ipeak++)
                for (int jhit = 0; jhit < int(vdet.size()); jhit++){
                    if (!valid(vdet[jhit].det, vdet[jhit].id)) continue;
                    for (int jpeak = 0; jpeak < int(vdet[jhit].twave.size()); jpeak++){
                        if ( ihit == jhit ) continue;
                        
                        double dt = vdet[ihit].twave[ipeak] - vdet[jhit].twave[jpeak];
                        if ( vdet[ihit].twave[ipeak] > flushlowedge && vdet[jhit].twave[jpeak] > flushlowedge )
                            hdt->Fill(dt);
                        
                        if ( abs(dt) > matrixwindow ) continue;
                        
                        h2xy->Fill(vdet[ihit].ewave[ipeak], vdet[jhit].ewave[jpeak]);
                    }
                }
        }
        
        
        
        if ( vdet.size() > 0 )
            opt->Fill();

        // display progress and time needed
        if (jentry != startentry){
            stop = clock();
            printf("Process %.3f %%  Time remaining %02d min %02d s                                     \r",double(jentry-startentry)/double(stopentry-startentry)*100.,
                int((stop-start)*(stopentry-jentry)/(jentry-startentry)/1e6/60),
                int((stop-start)*(stopentry-jentry)/(jentry-startentry)/1e6)%60);
            fflush(stdout);
        }

    }  // loop for entry

    stop = clock();
    printf("Process %.3f %%  Total Time %02d min %02d s        \n",100.,int((stop-start)/1e6/60),int((stop-start)/1e6)%60);
    
    opt->Write();
    
    // TH1/TH2 write
    hdt->Write();
    h2xy->Write();
}

void DataAnalysis::BranchOpt()
{
    br_vdet = 0;

    opt->Branch("det", &vdet);
}

// 
// event.cc ends here
