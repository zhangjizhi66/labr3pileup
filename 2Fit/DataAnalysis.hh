// DataAnalysis.hh --- 
// 

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

typedef struct DATAEVENT
{
    Short_t         det;//探测器类型
    Short_t         id;//探测器编号

    Short_t         sr;
    Long64_t        ts;
    UShort_t        raw;//原始能量道址
    double          e;//刻度能量
  
    Bool_t          pileup;
    Bool_t          outofr;
    Short_t         cfd;
    Bool_t          cfdft;
    Short_t         cfds;

#ifdef ENERGYSUM
    Bool_t esumf;
    UInt_t trae;  // trailing energy sum
    UInt_t leae;	// leading energy sum
    UInt_t gape;	// Gap energy sum
    UInt_t base;	// baseline value
#endif
  
#ifdef QDCSUM
    Bool_t qsumf;
    UInt_t qs[8];
#endif

#ifdef EXTERNALTS
    Bool_t etsf;
    Long64_t ets;
#endif

#ifdef WAVEFORM
    unsigned short ltra;
    // unsigned short *data;
    std::vector<unsigned short> data;
#endif
    
#ifdef SIMULATION
    std::vector<double> esim;
    std::vector<double> hsim;
    std::vector<double> tsim;
#endif

    // Long64_t        tsflag;
    // Short_t         sid;
    // Short_t         cid;
    // Short_t         ch;
    // UShort_t        evte;

} DATAEVENT;

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
    
#ifdef SIMULATION
    std::vector<double> esim;
    std::vector<double> hsim;
    std::vector<double> tsim;
#endif
  
} dets;

class DataAnalysis
{
public:
 
    DataAnalysis()
    {
        ipt = NULL;
        opt = NULL;
    }
    
    DataAnalysis(TTree *ipt_, TFile *ipf_wave_)
    {
        ipt = ipt_;
        ipf_wave = ipf_wave_;
        Init();
    }

    void Init();
    void Loop(TFile *opf_, TTree *opt_, Long64_t startentry, Long64_t stopentry);
    void BranchOpt();

private:
    std::vector<DATAEVENT>* br_event;
    TRandom3 gr;
    TTree *ipt;
    TTree *opt;
    TFile *ipf_wave;

    // --------------------------------------------
  
    // 0 ref, 

    dets det;
    std::vector<dets> vref, vdet;
};

#endif /* _DATAANALYSIS_H_ */
// 
// DataAnalysis.hh ends here

