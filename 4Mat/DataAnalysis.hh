// DataAnalysis.hh --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 一 9月 21 16:28:28 2020 (+0800)
// Last-Updated: 四 4月 27 21:47:23 2023 (+0800)
//           By: CIAE DAQ
//     Update #: 29
// URL: http://wuhongyi.cn 

#ifndef _DATAANALYSIS_H_
#define _DATAANALYSIS_H_

#include "../UserDefine.hh"

#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom3.h"
#include "TCutG.h"
#include "TGraph.h"
#include "TBenchmark.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

typedef struct dets
{
    Short_t  det;
    Short_t  id;
    Double_t e;
    Double_t raw;
    Long64_t ts;
    Double_t subts;
    Bool_t   pileup;
    Bool_t   outofr;

#ifdef WAVEFORM
    std::vector<float> wave;
#endif
  
    // add info
    std::vector<float> twave;
    std::vector<float> ewave;
    std::vector<int> nwave;
    std::vector<float> dtwave;
    std::vector<float> chi2ndfwave;
    std::vector<float> twavepre;
  
} dets;

class DataAnalysis
{
public:
 
    DataAnalysis()
    {
        ipt = NULL;
        opt = NULL;
    }
    
    DataAnalysis(TTree *ipt_, int run_)
    {
        ipt = ipt_;
        run = run_;
        Init();
    }

    void Init();
    void Loop(TFile *opf_, TTree *opt_, Long64_t startentry, Long64_t stopentry);
    void BranchOpt();

private:
    int run;
    
    std::vector<dets>* br_vdet;
    TRandom3 gr;
    TTree *ipt;
    TTree *opt;

    // --------------------------------------------
  
    // 0 ref, 

    dets det;
    std::vector<dets> vdet;
    
    double par[ndet][nid][2];
    
    TH1D *hdt;
    TH2D *h2xy;
};

#endif /* _DATAANALYSIS_H_ */
// 
// DataAnalysis.hh ends here

