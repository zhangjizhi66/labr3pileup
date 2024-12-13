// event.cc --- 
// 

#include "DataAnalysis.hh"
#include "../UserDefine.hh"

#include "TMath.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TDirectoryFile.h"
#include "TROOT.h"
#include "TMarker.h"

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
  
    // init TH1/TH2
    
    TDirectoryFile *dir_h2fwhm = new TDirectoryFile("h2fwhm","");
    
    for (int idet = 0; idet < ndet; idet++)
        for (int id = 0; id < nid; id++){
            if ( !valid(idet, id) ) continue;
            
            dir_h2fwhm->cd();
            h2fwhm[idet][id] = new TH2D(Form("h2fwhm_det%d_id%d",idet,id),"",500,0,maxoverflow,500,0,50);
            
            opf->cd();
            grfwhm[idet][id] = new TGraph;
            grfwhm[idet][id]->SetName(Form("grfwhm_det%d_id%d",idet,id));
            
            for (int ipnt = 0; ipnt < nfitpnt; ipnt++){
                grpnt[idet][id][ipnt] = new TGraph;
                grpnt[idet][id][ipnt]->SetName(Form("grpnt_det%d_id%d_ipnt%d",idet,id,ipnt));
            }
        }
}

void DataAnalysis::Loop(TTree *opt_)
{
    if (opt_ == NULL) return;

    opt = opt_;    
    BranchOpt();
    
    clock_t start = clock(), stop = clock();
  
    Long64_t startentry = 0, stopentry = ipt->GetEntries();
    for (Long64_t jentry = startentry; jentry < stopentry; jentry++){
        ipt->GetEntry(jentry);
       
        vdet.clear();

        for (int i = 0; i < int(br_event->size()); i++){
            det.det = (*br_event)[i].det;
            det.id = (*br_event)[i].id;
            det.raw = (*br_event)[i].raw;
            det.e = (*br_event)[i].e;
            det.ts = (*br_event)[i].ts;
            det.pileup = (*br_event)[i].pileup;
            det.outofr = (*br_event)[i].outofr;

            det.subts = 0.0;

            if ( (*br_event)[i].sr == 250 )
                det.ts = 8*det.ts;
            else if ( (*br_event)[i].sr == 100 || (*br_event)[i].sr == 500 )
                det.ts = 10*det.ts;

            if ( (*br_event)[i].sr == 500 )
                det.subts = ((*br_event)[i].cfds-1+(*br_event)[i].cfd/8192.0)*2.0;
            else if ( (*br_event)[i].sr == 250 )
                det.subts = ((*br_event)[i].cfd/16384.0-(*br_event)[i].cfds)*4.0;
            else if ( (*br_event)[i].sr == 100 )
                det.subts = ((*br_event)[i].cfd/32768.0)*10.0;

#ifdef WAVEFORM
            det.wave.clear();
            if ( (*br_event)[i].ltra > 0 ){     // !!!!!!
                det.wave.assign((*br_event)[i].data.begin(),(*br_event)[i].data.end());
                // det.wave.size()    det.wave[i]
            }
#endif
            //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

            vdet.push_back(det);
        }
        
        for (int ihit = 0; ihit < int(vdet.size()); ihit++){
            if ( int(vdet[ihit].wave.size()) <= 0 ) continue;
            
            if ( !valid(vdet[ihit].det, vdet[ihit].id) ) continue;
            
            // get baseline
            double base = 0;
            for (int ipnt = 0; ipnt < nbase; ipnt++)
                base += (double)vdet[ihit].wave[ipnt] / nbase;
            
            // wave
            TGraph *gwave = new TGraph;
            for (int ipnt = 0; ipnt < int(vdet[ihit].wave.size()); ipnt++)
                gwave->SetPoint(ipnt, ipnt, vdet[ihit].wave[ipnt] - base);
            
            // fastfilter
            
            std::vector<int> vtrig;

            int last1fastfilter = 0, last2fastfilter = 0;
            int thres = 0;
            TGraph *gfastfilter = new TGraph;
            for (int ipnt = 0; ipnt < int(vdet[ihit].wave.size()); ipnt++){
                int fastfilter = 0;
                if ( ipnt >= L && ipnt <= int(vdet[ihit].wave.size())-L ){
                    for (int jpnt = 0; jpnt < L; jpnt++)
                        fastfilter += vdet[ihit].wave[ipnt+jpnt] - vdet[ihit].wave[ipnt-L+jpnt];
                    fastfilter /= L;
                }
                
                gfastfilter->SetPoint(ipnt, ipnt, fastfilter);

                if ( ipnt < nbase ){
                    if ( fastfilter > thres )
                        thres = fastfilter;
                }
                else if ( ipnt == nbase )
                    thres *= fastfiltertrigger;
                else {
                    if ( fastfilter >= thres && last1fastfilter < thres && last2fastfilter < thres )
                        vtrig.push_back(ipnt);
                }
                
                last2fastfilter = last1fastfilter;
                last1fastfilter = fastfilter;
            }
            
            // mark trigger points
            for (int itrig = 0; itrig < int(vtrig.size()); itrig++){
                TMarker *mark = new TMarker(vtrig[itrig], gwave->GetPointY(vtrig[itrig]), 29);
                mark->SetMarkerColor(kRed);
                mark->SetMarkerSize(2);
                gwave->GetListOfFunctions()->Add(mark);
                //if (mark) delete mark;
            }
            
            for (int itrig = 0; itrig < int(vtrig.size()); itrig++){
                
                // pileup cut
                if ( itrig > 0 && vtrig[itrig] - vtrig[itrig-1] < length ) continue;
                if ( itrig < int(vtrig.size())-1 && vtrig[itrig+1] - vtrig[itrig] < length ) continue;
                
                // peaks
                int xmax = -1;
                double ymax = -1;
                for (int ipnt = vtrig[itrig] + trise/2; ipnt < int(vdet[ihit].wave.size()); ipnt++){
                    if ( vdet[ihit].wave[ipnt] - base > ymax ){
                        ymax = vdet[ihit].wave[ipnt] - base;
                        xmax = ipnt;
                    }
                    else if (ipnt - xmax > 5)
                        break;
                }
                
                // position cut
                if ( xmax < length || xmax > int(vdet[ihit].wave.size()) - length ) continue;

                // FWHM
                double FWHM1 = -1000, FWHM2 = -1000;
                for (int ipnt = vtrig[itrig]; ipnt < int(vdet[ihit].wave.size()); ipnt++){
                    if ( vdet[ihit].wave[ipnt]-base <= ymax/2 && vdet[ihit].wave[ipnt+1]-base > ymax/2 )
                        FWHM1 = ((vdet[ihit].wave[ipnt+1]-base-ymax/2)*ipnt-(vdet[ihit].wave[ipnt]-base-ymax/2)*(ipnt+1))/(vdet[ihit].wave[ipnt+1]-vdet[ihit].wave[ipnt]);
                    if ( vdet[ihit].wave[ipnt]-base >= ymax/2 && vdet[ihit].wave[ipnt+1]-base < ymax/2 )
                        FWHM2 = ((vdet[ihit].wave[ipnt+1]-base-ymax/2)*ipnt-(vdet[ihit].wave[ipnt]-base-ymax/2)*(ipnt+1))/(vdet[ihit].wave[ipnt+1]-vdet[ihit].wave[ipnt]);
                    if (FWHM1 > 0 && FWHM2 > 0) break;
                }
                
                h2fwhm[vdet[ihit].det][vdet[ihit].id]->Fill(ymax, FWHM2-FWHM1);
                grfwhm[vdet[ihit].det][vdet[ihit].id]->SetPoint(grfwhm[vdet[ihit].det][vdet[ihit].id]->GetN(), ymax, FWHM2-FWHM1);
                for (int ipnt = 0; ipnt < nfitpnt; ipnt++){
                    grpnt[vdet[ihit].det][vdet[ihit].id][ipnt]->SetPoint(grpnt[vdet[ihit].det][vdet[ihit].id][ipnt]->GetN(), ymax, (vdet[ihit].wave[xmax+ipnt-length]-base) / ymax);
                }
            }

            // output

            if (jentry < 10){
                gwave->SetName(Form("gwave_%d_%d_%lld_%d", vdet[ihit].det, vdet[ihit].id, jentry, ihit));
                gwave->SetTitle(Form("wave det=%d id=%d raw=%.1f", vdet[ihit].det, vdet[ihit].id, vdet[ihit].raw));
                gwave->Write();
                    
                gfastfilter->SetName(Form("gfastfilter_%d_%d_%lld_%d", vdet[ihit].det, vdet[ihit].id, jentry, ihit));
                gfastfilter->SetTitle(Form("fastfilter det=%d id=%d raw=%.1f", vdet[ihit].det, vdet[ihit].id, vdet[ihit].raw));
                gfastfilter->Write();
            }

            delete gwave;
            delete gfastfilter;
        }
        
        //if ( vdet.size() > 0 )
            //opt->Fill();

        // display progress and time needed
        if (jentry%100 == 99){
            stop = clock();
            printf("Process %.3f %%  Time remaining %02d min %02d s                                     \r",double(jentry-startentry)/double(stopentry-startentry)*100.,
                int((stop-start)*(stopentry-jentry)/(jentry-startentry)/1e6/60),
                int((stop-start)*(stopentry-jentry)/(jentry-startentry)/1e6)%60);
            fflush(stdout);
        }
    }  // loop for entry
    stop = clock();
    printf("Process %.3f %%  Total Time %02d min %02d s        \n",100.,int((stop-start)/1e6/60),int((stop-start)/1e6)%60);
    
    // TH1/TH2 write  opt->Write();
    
    for (int idet = 0; idet < ndet; idet++)
        for (int id = 0; id < nid; id++){
            if ( !valid(idet,id) ) continue;
            
            //h2fwhm[idet][id]->Write();
            grfwhm[idet][id]->Write();
            for (int ipnt = 0; ipnt < nfitpnt; ipnt++)
                grpnt[idet][id][ipnt]->Write();
        }
        
}

void DataAnalysis::BranchOpt()
{
    br_event = 0;

    opt->Branch("det", &vdet);
}

// 
// event.cc ends here
