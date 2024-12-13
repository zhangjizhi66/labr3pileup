// Last Modified zhangjizhi, 20220503

// Generate background spectrum and gg-matrix using Radware approch.
// Author: Li Zhihuan, Peking University

//> root -l gate2matrix.C
//root[1] genBackground() //generate background spectrum
//root[2] gate2mat() // gg-matrix

#include "../UserDefine.hh"

#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TROOT.h"
#include "TSpectrum.h"
#include <iostream>
#include <ctime>

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */

using namespace std;

//input 
int run = 117;
TString fInName = Form("%s%s_%04d_W%d.root",FILEPATH,MATFILENAME,run,EVENTTIMEWINDOWSWIDTH);//root file 
TString h2name = "hg2xy";//gg-matrix 
//output 
TString fOutName = Form("%sgate2matrix_%04d.root",FILEPATH,run);//root file

////////////////////////////////////////////////////////
TH2F *hg2xy=NULL;
int nbin; 
float xmin;
float xmax;

//1d total projection
TH1F *hg2x=NULL;
TH1F *hg2xb=NULL;
TH1F *hg2xp=NULL;

//matrix
TH2F *hg2xyb=NULL;
TH2F *hg2xyp=NULL;

void genBackground(int res=15,int nosmoothing=1);
void write();

void gate2matrix()
{
  TFile *fin = new TFile(fInName.Data());
  if ((!fin) || (fin->IsZombie())) {
    cout<<fInName.Data()<<" does not exist!"<<endl;
    return;
  }
  hg2xy = (TH2F*)fin->Get(h2name.Data()); //original gamma-gamma matrix
  if (!hg2xy) return;
  
  nbin = hg2xy->GetNbinsX();
  xmin = hg2xy->GetXaxis()->GetBinLowEdge(1);
  xmax = hg2xy->GetXaxis()->GetBinLowEdge(nbin+1);
  
  //1d total projection
  hg2x = (TH1F*)hg2xy->ProjectionX("hg2x",1,nbin);//total projection spectrum
  cout<<nbin<<","<<xmin<<","<<xmax<<endl;
  
  //matrix
  hg2xyb = new TH2F("hg2xyb","bg matrix",nbin,xmin,xmax,nbin,xmin,xmax);//background matrix
  hg2xyp = new TH2F("hg2xyp","bg subtracted matrix",nbin,xmin,xmax,nbin,xmin,xmax);//background subtracted matrix
  
  genBackground();
    
  cout<<'\n'<<RED<<"(1) If the shape of the background spectrum looks OK, then run write() "<<'\n';
  cout<<"(2) If not then run genBackground(res) with modified res value until the (1) is meet." <<RESET<<'\n'<<'\n';
}

void genBackground(int res, int nosmoothing)
{
  if (hg2xb!=NULL) delete hg2xb;
  if (hg2xp!=NULL) delete hg2xp;

  TSpectrum *sa = new TSpectrum(1000);
  if (nosmoothing)
      hg2xb = (TH1F*)sa->Background(hg2x,res,"nosmoothing");//background spectrum, user
  else
      hg2xb = (TH1F*)sa->Background(hg2x,res);
  hg2xp = new TH1F("hg2xp","",nbin,xmin,xmax);//peak spectrum
  hg2x->Sumw2(0);
  hg2x->Draw();
  hg2xb->Draw("same");
  hg2xp->Add(hg2x,hg2xb,1,-1); 
}

void write()
{
  //make background matrix - RADWARE approch
  Double_t T,Pi,Pj,pi,pj,Bij;
  T = hg2x->Integral();
  int N=hg2x->GetNbinsX();
  for (int i=1; i<=N; i++)
    for (int j=1; j<=N; j++){
      Pi = hg2x->GetBinContent(i);
      Pj = hg2x->GetBinContent(j);
      pi = hg2xp->GetBinContent(i);
      pj = hg2xp->GetBinContent(j);
      Bij = (Pi*Pj-pi*pj)/T;
      Double_t x = hg2x->GetBinCenter(i);
      Double_t y = hg2x->GetBinCenter(j);       
      hg2xyb->Fill(x,y,Bij);
    }
  hg2xyp->Sumw2(0);
  hg2xyp->Add(hg2xy,hg2xyb,1,-1);

  //output to ROOT file
  TFile *fout = new TFile(fOutName.Data(),"RECREATE");//user
  hg2xy->SetName("hg2xy");
  hg2xy->Write();
  hg2xyp->Write();
  hg2x->Write();
  hg2xyb->Write();
  fout->Close();
  cout<<"Write Histgrams hg2xyp, hg2x and hg2xb to "<<fOutName.Data()<<endl;
  gROOT->ProcessLine(".q");
}
