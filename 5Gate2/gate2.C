// Last Modified zhangjizhi, 20220609

#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TList.h"
#include "TString.h"
#include "TSpectrum.h"
#include "TLatex.h"
#include "TColor.h"
#include "TPolyMarker.h"
#include "TApplication.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TRootCanvas.h"
#include "TLine.h"
#include "TVirtualX.h"

#include <map>
#include <algorithm>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#define codename         "Gamma-Gamma two-fold matrix data analysis Code GATE2"
#define author           "Author: Zhihuan Li, Peking University"
#define lastmodified     "Last modified on Dec. 4, 2021"
#define webpage          "Find the last updates from [https://github.com/zhihuanli/gamma-gamma-coincidence-analysis/tree/master/Gate2]"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */                                                                
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */

struct parameters
{
  TString rootfile;
  std::vector<TString> histname;
  int     ncanvas;
  int     xmin;    //range for hist
  int     xmax;    //range for hist
  int     npeaks;  //number of peaks for TSpectrum
  float   dge;    //gate width:ge-dge - ge+dge

};
parameters st;

TString GateAxis = "X";
int OpenCross = 1;
int deploymark = 1;

std::map<TString,int> mg;
std::map<TString,TString> mgtitle;
std::map<TString,int>::iterator img;
std::map<TString,TString>::iterator imgtitle;

TList *tlhist = nullptr, *tlcanv = nullptr;

int xmin = 0;
int xmax;

TFile *f = nullptr;
TH1 *hg2x = nullptr;
TH1 *hg2y = nullptr;
TH2 *hg2xy = nullptr;
std::vector<TH2*> hg2xys;

TCanvas *ca[1000];
int ic = -1;//canvas id
int icy = 1;
int ncy = 1;//number of windows in y axia
int ih = 0;

void setncanvas(int ncanvas = 5) { st.ncanvas = ncanvas; }
void setxrange(int xmin1 = xmin, int xmax1 = xmax) { st.xmin = xmin1; st.xmax = xmax1;};
void setnpeaks(int npeaks1 = 30) {st.npeaks = npeaks1;};
void setgatewidth(float dgea = -2, float dgeb = 2) {st.dge = dgea;};
void showSettings();
// Show different parts of a gated-spectrum in the same window
void gs(float ge1,int npad = 4);
void gsxr(int xmin, int xmax, float ge1, int npad = 4);
void gscp(float ge1);
//gated-spectra up to six peaks - draw existing histograms
void gm(float ge1, float ge2 = -1000, float ge3 = -1000, float ge4 = -1000);
void gm(TH1* h1, TH1 *h2 = nullptr, TH1 *h3 = nullptr, TH1 *h4 = nullptr);
void gmxr(int xmin, int xmax, float ge1, float ge2 = -1000, float ge3 = -1000, float ge4 = -1000);
void gmxr(int xmin, int xmax, TH1* h1, TH1 *h2 = nullptr, TH1 *h3 = nullptr, TH1 *h4 = nullptr);
//total projection sepctra
void tpjm(int npad = 3);
void tpjmxr(int xmin, int xmax, int npad = 3);
//two-fold gated with a specified gamma peak range
void gw(float ge1, float ge2);
void gwxr(int xmin, int xmax, float ge1, float ge2);
//and
void gand(float ge1, float ge2, float ge3 = -1000, float ge4 = -1000);
void gandxr(int xmin, int xmax, float ge1, float ge2, float ge3 = -1000, float ge4 = -1000);
//add 
void gadd(float ge1, float ge2, float ge3 = -1000, float ge4 = -1000, float ge5 = -1000, float ge6 = -1000);
void gaddxr(int xmin, int xmax, float ge1, float ge2, float ge3 = -1000, float ge4 = -1000, float ge5 = -1000, float ge6 = -1000);
//sub
void gsub(float ge1, float ge2, float ge3 = -1000, float ge4 = -1000);
void gsubxr(int xmin, int xmax, float ge1, float ge2, float ge3 = -1000, float ge4 = -1000);
//zoom 2-D histogram
void zoom(Double_t x, Double_t y, Double_t wx = 200, Double_t wy = 200, TString sname = st.histname[1]);

//help
void help();


//show all existing gated histograms
void histlist(){
  std::cout<<GREEN<<std::endl;
  tlhist->ls();
  std::cout<<RESET<<std::endl;
 };//show name of all histograms

//show all existing TCanvas
void canvlist(){
  std::cout<<GREEN<<std::endl;
  tlcanv->ls();
  std::cout<<RESET<<std::endl;
 };

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//private functions
bool ReadSettings(TString fname);
void quit();
//two-fold gated
TString g(double ge = 0,int icy1 = 1,TH2 *hg2xyp = hg2xy);
void newcanvas(int ncy, TString sctitle);
TString GetHistName(TString sprefix);
void tpj(int icy = 1);
//mark peak positions
void peaks(TH1 *h, Double_t thres = 0.05);
void DynamicCross(int nPad);
void AxisSync(int nPad);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void gate2()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
  std::cout<<"This code is valid only for  ROOT6 !"<<std::endl;
  gApplication->Terminate(0);
#endif
#ifdef R__WIN32
  std::cout<<"win32"<<std::endl;
#endif
  
#ifdef R__UNIX
  std::cout<<"LINUX"<<std::endl;
#endif

#ifdef R__HAS_COCOA
  std::cout<<"MAC COCOA"<<std::endl;
#endif

  // std::cout<<gStyle->GetCanvasPreferGL()<<std::endl;
  //为了保证流畅度，关闭 OPENGL。代价就是 SetFillColorAlpha 函数无法使用
  gStyle->SetCanvasPreferGL(false);
  
  TColor::SetColorThreshold(0.1);
  std::cout<<std::endl;
  std::cout<< CYAN<<codename <<std::endl;
  std::cout<< BLUE<<author <<std::endl;
  std::cout<< BLUE<<lastmodified <<std::endl;
  std::cout<< BLUE<<webpage <<std::endl;
  std::cout<< RESET<<std::endl;
  bool bvalid = ReadSettings("gate2set.txt");
  if(!bvalid) quit();
  if(gSystem->AccessPathName(st.rootfile.Data())){
    std::cout <<RED<<"ROOT file: "<<st.rootfile.Data()<<" does not exist" <<RESET<< std::endl;
    quit();
  } 
  TFile *f = new TFile(st.rootfile.Data());
  if(!(hg2xy = (TH2*)f->Get(st.histname[0].Data()))) {
    std::cout << RED << "ERROR!  No 2D-histogram in " << st.rootfile.Data() << " is detected !" << RESET << std::endl;
    quit();
  }
  hg2x = (TH1*)hg2xy->ProjectionX("hg2x");
  hg2y = (TH1*)hg2xy->ProjectionY("hg2y");
    
  int nhist = st.histname.size();
  for (int i = 0; i < nhist; i++){
      TH2 *hptemp = (TH2*)f->Get(st.histname[i].Data());
      hg2xys.push_back(hptemp);
  }
  
  xmax = hg2x->GetBinCenter(hg2x->GetXaxis()->GetNbins());
  
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
  setncanvas(st.ncanvas);
  setxrange(st.xmin,st.xmax);
  setnpeaks(st.npeaks);
  setgatewidth(st.dge);
  showSettings();
  std::cout<<RED<<"help() for short instructions "<<RESET<<std::endl<<std::endl;
  
  if(tlhist == nullptr) tlhist = new TList();
  if(tlcanv == nullptr) tlcanv = new TList();  
  tpjm();
  // if(aae!=NULL) gaem();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Float_t gethx(Float_t x)
{
  int bx=hg2x->FindBin(x);
  int minbx=1;
  int maxbx=hg2x->GetNbinsX();
  int by=hg2y->FindBin(x);
  int minby=1;
  int maxby=hg2y->GetNbinsX();

  
  if(GateAxis=="X") {
    if(bx<minbx || bx>maxbx) {
          std::cout<<RED<<"peak value is out of range! "<<RESET<<std::endl;
	  return -1;
    }
    return hg2x->GetBinLowEdge(bx);
  }
  if(GateAxis=="Y") {
    if(by<minby || by>maxby) {
          std::cout<<RED<<"peak value is out of range! "<<RESET<<std::endl;
	  return -1;
    }
    return hg2y->GetBinLowEdge(by);
  }
  return -1;
  
}

void tpjm(int npad)
{
  if(npad > 6) npad = 6;
  newcanvas(npad,GateAxis+" Axis Total Projection Spectrum");
  for(int i = 1; i <= npad; i++) {
    ca[ic]->GetPad(i)->DeleteExec("AxisSync");
  }
  int xmina = st.xmin;
  int xmaxa = st.xmax;
  int dx = (xmaxa-xmina)/npad;
  for(int i = 0; i < npad; i++) {
    setxrange(xmina+i*dx, xmina+(i+1)*dx);
    tpj(i+1);
  }
  setxrange(xmina,xmaxa);
  
}
void tpjmxr(int xmin, int xmax, int npad)
{
  int x1 = st.xmin;
  int x2 = st.xmax;
  setxrange(xmin,xmax);
  tpjm(npad);
  st.xmin = x1;
  st.xmax = x2;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void gsxr(int xmin, int xmax, float ge1, int npad)
{
  int x1 = st.xmin;
  int x2 = st.xmax;
  setxrange(xmin, xmax);
  gs(ge1, npad);
  st.xmin = x1;
  st.xmax = x2;
}

void gs(float ge1, int npad)
{
  if(npad > 6) npad = 6;
  ge1=gethx(ge1);
  if(ge1<0) return;
  TString sctitle = Form("gs(%.1f)",ge1);
  newcanvas(npad,sctitle);
  for(int i = 1; i <= npad; i++) {
    ca[ic]->GetPad(i)->DeleteExec("AxisSync");
  }
  int xmina = st.xmin;
  int xmaxa = st.xmax;
  int dx = (xmaxa-xmina)/npad;
  std::cout<<dx<<std::endl;
  for(int i = 0; i < npad; i++) {
    int x1 = xmina+i*dx;
    int x2 = xmina+(i+1)*dx;
    setxrange(x1,x2);
    g(ge1,i+1);
  }
  setxrange(xmina,xmaxa);
}
void gscp(float ge1)
{
  ge1=gethx(ge1);
  if(ge1<0) return;  
  if (st.histname.size() > 4){
    std::cout<<RED<<"ERROR! Too many matrixes !"<<RESET<<std::endl;
    return;
  }
    
  int npad = st.histname.size();
  for (int i = 0; i < npad; i++)
      if (!hg2xys[i]){
          cout<<RED<<"ERROR!  No 2D-histogram "<<st.histname[i].Data()<<" in "<<st.rootfile.Data()<<" is detected !"<<RESET<<endl;
          return;
      }
  ge1=gethx(ge1);
  TString sctitle = Form("gscp(%.1f)",ge1);
  newcanvas(npad,sctitle);
  for (int i = 0; i < npad; i++)
      g(ge1,i+1,hg2xys[i]);
}
void gwxr(int xmin,int xmax,float ge1,float ge2)
{
  int x1 = st.xmin;
  int x2 = st.xmax;
  setxrange(xmin, xmax);
  gw(ge1, ge2);
  st.xmin = x1;
  st.xmax = x2;
}
void gw(float ge1,float ge2)
{
  if(ge1 < xmin) ge1 = xmin;
  if(ge2 > xmax) ge2 = xmax;
  ge1=gethx(ge1);
  ge2=gethx(ge2);
  if(ge1<0 || ge2<0 ) return;
  if(ge1>=ge2) {
    cout<<"wrong parameters!"<<endl;
    return;
  }
  TString sctitle = Form("gw(%.1f,%.1f)",ge1, ge2);  
  newcanvas(1, sctitle);
  TString stitle = Form("Gate: %.1f-%.1f keV", ge1, ge2);
  int gea = hg2xy->GetXaxis()->FindBin(ge1);
  int geb = hg2xy->GetXaxis()->FindBin(ge2);

  TString sname = GetHistName(Form("gw%d-%d", int(ge1), int(ge2)));
  TH1F *ha = (TH1F*)hg2xy->ProjectionX(sname.Data(), gea, geb);
  ha->SetTitle(stitle);
  if(!tlhist->FindObject(ha)) tlhist->Add(ha);
  peaks(ha);
}

void gmxr(int xmin,int xmax,float ge1,float ge2,float ge3,float ge4)
{
  int x1 = st.xmin;
  int x2 = st.xmax;
  setxrange(xmin,xmax);
  gm(ge1,ge2,ge3,ge4);
  st.xmin = x1;
  st.xmax = x2;

}

void gm(float ge1,float ge2,float ge3,float ge4)
{
  int npad = 0;
  double ge[4] = {ge1, ge2, ge3, ge4};
  for(int i = 0; i < 4; i++) {
    if(ge[i]<-999) break;
    ge[i]=gethx(ge[i]);
    if(ge[i]<0) return;
    npad++;
  }
  
  TString sctitle = Form("gm(%.1f", ge[0]);
  for (int i = 1; i < npad; i++) 
    sctitle += Form(",%.1f", ge[i]);
  sctitle += ")";
  newcanvas(npad, sctitle);
  for(int i = 0; i < npad; i++)
    g(ge[i], i+1);
}

void gmxr(int xmin, int xmax, TH1* h1, TH1 *h2, TH1 *h3, TH1 *h4)
{
  int x1 = st.xmin;
  int x2 = st.xmax;
  setxrange(xmin,xmax);
  gm(h1, h2, h3, h4);
  st.xmin = x1;
  st.xmax = x2;

}

void gm(TH1* h1, TH1 *h2, TH1 *h3, TH1 *h4)
{
  int npad = 0;
  TH1 *hg[4] = {h1, h2, h3, h4};
  std::map<int,int> mg[4];
  for(int i = 0; i < 4; i++) 
    if(hg[i] != nullptr) npad++;
  TString sctitle = Form("gm(%s",hg[0]->GetName());
  for(int i = 1; i < 4; i++) 
    if(hg[i] != nullptr) sctitle += Form(",%s",hg[i]->GetName());
  sctitle += ")";
  newcanvas(npad,sctitle);

  for(int i = 0; i < npad; i++) {
    ca[ic]->cd(i+1);
    peaks(hg[i]);
  }
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void gandxr(int xmin,int xmax,float ge1, float ge2, float ge3, float ge4)
{
  int x1 = st.xmin;
  int x2 = st.xmax;
  setxrange(xmin, xmax);
  gand(ge1, ge2, ge3, ge4);
  st.xmin = x1;
  st.xmax = x2;

}
void gand(float ge1, float ge2, float ge3, float ge4)
{

  int npad = 0;
  TH1F *ha[4];
  double ge[4] = {ge1,ge2,ge3,ge4};
  for(int i = 0; i < 4; i++) {
    if(ge[i]<-999) break;
    ge[i]=gethx(ge[i]);
    if(ge[i]<0) return;
    npad++;
  }
  
  TString sctitle = Form("gand(%.1f", ge[0]);
  for(int i = 1; i < 4; i++) 
    sctitle += Form(",%.1f", ge[i]);
  sctitle += ")";
  
  newcanvas(npad+1, sctitle);

  TString stitle = "Gate: ";
  for(int i = 0; i < npad; i++) {
    TString hname = g(ge[i], i+1);//return name of histogram.
    ha[i] = (TH1F*)gROOT->FindObject(hname);
    stitle = Form("%s %.1f", stitle.Data(), ge[i]);
    if(i<npad-1) stitle=Form("%s && ", stitle.Data());
  }
  
  stitle = Form("%s keV", stitle.Data());
  TString sname = GetHistName("gand");  
  TH1F *hand = (TH1F*)hg2xy->ProjectionX(sname.Data(), xmin, xmax);//copy hist structure
  hand->SetTitle(stitle);
  hand->Reset("M");//!!! reset maximum and minimum;
  for(int i = 1; i <= ha[0]->GetNbinsX(); i++){
    double ma = ha[0]->GetBinContent(i);
    for(int j = 1; j < npad; j++) {
      double mb = ha[j]->GetBinContent(i);
      ma = TMath::Min(ma,mb);
    }
    hand->SetBinContent(i,ma);
  }
  if(!tlhist->FindObject(hand)) tlhist->Add(hand);
 
  ca[ic]->cd(npad+1);
  icy = npad+1;
  peaks(hand);
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void gaddxr(int xmin, int xmax, float ge1, float ge2, float ge3, float ge4, float ge5, float ge6)
{
  int x1 = st.xmin;
  int x2 = st.xmax;
  setxrange(xmin, xmax);
  gadd(ge1, ge2, ge3, ge4, ge5, ge6);
  st.xmin = x1;
  st.xmax = x2;

}
void gadd(float ge1, float ge2, float ge3, float ge4, float ge5, float ge6)
{
  int npad = 0;
  TH1F *ha[6];
  double ge[6] = {ge1, ge2, ge3, ge4, ge5, ge6};
  for(int i = 0; i < 6; i++) {
    if(ge[i]<-999) break;
    ge[i]=gethx(ge[i]);
    if(ge[i]<0) return;
    npad++;
  }
  
  TString sctitle = Form("gadd(%.1f", ge[0]);
  for(int i = 1; i < npad; i++) 
   sctitle += Form(",%.1f", ge[i]);
  sctitle += ")";
  newcanvas(npad+1, sctitle);
  TString stitle = "Gate: ";
  
  for(int i = 0; i < npad; i++) {
    TString hname = g(ge[i], i+1);//return name of histogram.
    ha[i] = (TH1F*)gROOT->FindObject(hname);
    stitle = Form("%s %.1f", stitle.Data(), ge[i]);
    if(i<npad-1) stitle = Form("%s + ", stitle.Data());
  }
  stitle = Form("%s keV", stitle.Data());
  TString sname = GetHistName("gadd");    
  TH1F *hadd = (TH1F*)hg2xy->ProjectionX(sname.Data(), xmin, xmax);//copy hist structure
  hadd->SetTitle(stitle);
  hadd->Reset("M");//!!! reset maximum and minimum;

  for(int i = 0; i < npad; i++) hadd->Add(hadd,ha[i]);
   if(!tlhist->FindObject(hadd)) tlhist->Add(hadd);
  ca[ic]->cd(npad+1);
  icy = npad+1;
  peaks(hadd);
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void gsubxr(float ge1, float ge2, float ge3, float ge4)
{
  int x1 = st.xmin;
  int x2 = st.xmax;
  setxrange(xmin,xmax);
  gsub(ge1, ge2, ge3, ge4);
  st.xmin = x1;
  st.xmax = x2;  
}
void gsub(float ge1, float ge2, float ge3, float ge4)
{
  int npad = 0;
  TH1F *ha[4];
  double ge[4] = {ge1, ge2, ge3, ge4};
  for(int i = 0; i < 4; i++) {
    if(ge[i]<-999) break;
    ge[i]=gethx(ge[i]);
    if(ge[i]<0) return;
    npad++;
  }
  
  TString sctitle = Form("gsub(%.1f", ge[0]);
  for(int i = 1; i < npad; i++) 
   sctitle += Form(",%.1f", ge[i]);
  sctitle += ")";
  
  newcanvas(npad+1, sctitle);
  TString stitle = "Gate: ";
  for(int i = 0; i < npad; i++) {
    TString hname = g(ge[i], i+1);//return name of histogram.
    ha[i] = (TH1F*)gROOT->FindObject(hname);
    stitle = Form("%s %.1f", stitle.Data(), ge[i]);
    if(i<npad-1)  stitle = Form("%s - ", stitle.Data());
  }
  stitle = Form("%s keV", stitle.Data());
  TString sname = GetHistName("gsub");    
  TH1F *hsub = (TH1F*)hg2xy->ProjectionX(sname.Data(), xmin, xmax);//copy hist structure

  hsub->SetTitle(stitle);
  hsub->Reset("M");//!!! reset maximum and minimum;

  for(int i = 1; i < npad; i++) hsub->Add(ha[0], ha[i], 1, -1);
  if(!tlhist->FindObject(hsub)) tlhist->Add(hsub);
  ca[ic]->cd(npad+1);
  icy = npad+1;
  peaks(hsub);
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void zoom(Double_t x, Double_t y, Double_t wx, Double_t wy, TString sname)
{
    TH2 *h2 = (TH2*)gROOT->FindObject(sname.Data());
    if (!h2 || sname.IsNull()){
      std::cout<<RED<<"ERROR!  No 2D-histogram "<<sname.Data()<<" is detected ! Specify the 2nd histname as no background subtracted matrix in gate2set.txt !"<<RESET<<std::endl;
          return;
    }
    h2->SetAxisRange(x-wx/2,x+wx/2, "X");
    h2->SetAxisRange(y-wy/2,y+wy/2, "Y");
    h2->SetStats(0);
    TString sctitle = Form("zoom(%.1f,%.1f)", x, y);
    newcanvas(1, sctitle);
    ca[ic]->SetWindowSize(800,800);
    h2->Draw("colz");
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void saveHist(TH1* hist, TString sfile)
{
  TFile *fout = new TFile(sfile.Data(), "UPDATE");
  hist->Write();
  fout->Close();
  std::cout<<"Write "<<hist->GetName()<<" to "<<sfile.Data()<<std::endl;
}

void saveHistAll(TString sfile)
{
  TFile *fout = new TFile(sfile.Data(), "UPDATE");
  tlhist->Write();
  fout->Close();
  std::cout<<"Write all existing histograms to "<<sfile.Data()<<std::endl;  
}

void saveHistData(TH1* hist, TString sfile)
{
  std::ofstream fout(sfile.Data());
  for(int i = 1; i <= hist->GetNbinsX(); i++) {
    fout<<Form("%.1f   %.1f", hist->GetBinCenter(i), hist->GetBinContent(i)) << std::endl;
  }
  fout.close();
  std::cout<<"Write "<<hist->GetName()<<" to "<<sfile.Data()<<std::endl;
}

void saveCanv(TCanvas* c1, TString sfile)
{
  TFile *fout=new TFile(sfile.Data(),"UPDATE");
  c1->Write();
  fout->Close();
  std::cout<<"Write "<<c1->GetName()<<" to "<<sfile.Data()<<std::endl;
}

void saveCanvAll(TString sfile)
{
  TFile *fout = new TFile(sfile.Data(), "UPDATE");
  TIter next(tlcanv);
  while (TCanvas* cc = (TCanvas*)next()) cc->Write();
  //tlcanv->Write();//some error messages when browse through TCanvas in the root file in MacOS.
  fout->Close();
  std::cout<<"Write all existing histograms to "<<sfile.Data()<<std::endl;  
}


void showSettings()
{
  std::cout<<GREEN<<"ROOT file: " <<st.rootfile.Data()<<std::endl;
  std::cout<<GREEN<<"2D-histogram: ";
  for (TString ts : st.histname)
    std::cout<<ts.Data()<<" ";
  std::cout<<std::endl;
  std::cout<<GREEN<<Form("Range of X/Y: %d - %d ",xmin,xmax)<<RESET<<std::endl<<std::endl;
  std::cout<<RED<<"Current settings:"<<std::endl;
  std::cout<<CYAN<<"ncanvas = "<<st.ncanvas<<std::endl;
  std::cout<<CYAN<<"xmin = "<<st.xmin<<", xmax= "<<st.xmax<<std::endl;
  std::cout<<CYAN<<"npeaks = "<<st.npeaks<<std::endl;
  std::cout<<CYAN<<"dge = "<<st.dge<<std::endl;
  std::cout<<RESET<<std::endl;
}


void quit()
{
  gApplication->Terminate(0);
}

void help()
{
  cout<<YELLOW<<"....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......"<<endl;
  cout<< RED<<"Setup, "<<RED<<"changes will take effect for the next drawing."<<endl;
  cout<< CYAN<< "   setxrange(int xmin1, int xmax1)        - Change the viewing range of x-axis for all histograms."<<endl;
  cout<< "   setnpeaks(int npeaks1=30)              - Change the number of peaks marked in a histogram."<<endl;
  cout<< "   setncanvas(int ncanvas=5)              - Change the maximum number of canvas avaliable."<<endl;
  cout<< "   setgatewidth(int dge=2)                - Change range of gate to peak-dge to peak+dge."<<endl;
  cout<< "   showSettings()                         - Show current parameter settings."<<endl;
  
  std::cout<<YELLOW<<"....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......"<<std::endl;
  std::cout<<RED<<"Show a gated-spectrum."<<std::endl;
  std::cout<<CYAN<<"   gs(float ge1,int npad=4)"<<std::endl;
  std::cout<<RED<<"Show gated-spectra"<<std::endl;
  std::cout<<CYAN<<  "   gm(float ge1, [float ge2], [float ge3], [float ge4])"<<std::endl;
  std::cout<<CYAN<< "   gm(TH1* h1, [TH1* h2],[TH1* h3],[TH1* h4])"<<std::endl;
  std::cout<<RED<< "Show total projection sepctrum."<<std::endl;
  std::cout<<CYAN<< "   tpjm(int npad)"<<std::endl;
  std::cout<<RED<< "Show a spectrum gated from ge1 to ge2"<<std::endl;
  std::cout<<CYAN<< "   gw(float ge1,float ge2)"<<std::endl;
  std::cout<<RED<< "Show AND, ADD, SUB gated spectra"<<std::endl;
  std::cout<<CYAN<<"   gand(float ge1,float ge2, [float ge3], [float ge4])"<<std::endl;
  std::cout<<"   gadd(float ge1,float ge2, [float ge3], [float ge4], [float ge5], [float ge6])"<<std::endl;
  std::cout<<"   gsub(float ge1,float ge2, [float ge3],[float ge4])"<<std::endl;

  std::cout<<RED<<"Show gated spectra with specified the x-axis range."<<std::endl;
  std::cout<<CYAN<<"   gsxr(int xmin,int xmax,float ge1,int npad=4)"<<std::endl;
  std::cout<<CYAN<<  "   gmxr(int xmin,int xmax,double ge1, [double ge2], ...)"<<std::endl;
  std::cout<<CYAN<<  "   gmxr(int xmin,int xmax,TH1* h1, [TH1* h2], ...)"<<std::endl;
  std::cout<<CYAN<< "   tpjmxr(int xmin,int xmax,int npad)"<<std::endl;
  std::cout<<CYAN<< "   gwxr(int xmin,int xmax,float ge1,float ge2)"<<std::endl;
  std::cout<<CYAN<<"   gandxr(int xmin,int xmax,float ge1,float ge2, [float ge3], [float ge4])"<<std::endl;
  std::cout<<"   gaddxr(int xmin,int xmax,float ge1,float ge2, [float ge3], [float ge4], [float ge5], [float ge6])"<<std::endl;
  std::cout<<"   gsubxr(int xmin,int xmax,float ge1,float ge2, [float ge3],[float ge4])"<<std::endl; 


  std::cout<<YELLOW<<"....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......"<<std::endl;
  std::cout<<RED<<"Save a hist to a ROOT file with append mode"<<std::endl;
  std::cout<<CYAN<<"    saveHist(TH1* hist, TString sfile)"<<std::endl;
  std::cout<<RED<<"Save a canvas to a ROOT file with append mode"<<std::endl;
  std::cout<<CYAN<<"    saveCanv(TCanvas* c1, TString sfile)"<<std::endl;
  std::cout<<RED<<"Save a hist data to a text file"<<std::endl;
  std::cout<<CYAN<<"    saveHistData(TH1* c1, TString sfile)"<<std::endl;
  std::cout<<RED<<"Save All histograms to a ROOT file"<<std::endl;
  std::cout<<CYAN<<"    saveHistAll(TString sfile)"<<std::endl;
  std::cout<<RED<<"Save All TCanvas to a ROOT file"<<std::endl;
  std::cout<<CYAN<<"    saveCanvAll(TString sfile)"<<std::endl;
  
  std::cout<<RED<<"Show all existing gated histograms"<<std::endl;
  std::cout<<CYAN<<"   histlist()"<<std::endl;

  std::cout<<RED<<"Show all existing TCanvas"<<std::endl;
  std::cout<<CYAN<<"   canvlist()"<<std::endl;

  std::cout<<RED<<"Safe way to quit the program in mac."<<std::endl;
  std::cout<<CYAN<<"   quit()"<<std::endl;
    
  std::cout<<RESET<<std::endl;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//private functions
bool ReadSettings(TString fname)
{
  //#### Gate2 initial settings ####
  //rootfile    hgglmatrix.root 
  //histname    hg2xyp hg2xy hg2xyb
  //ncanvas     5              
  //xmin        0
  //xmax        4000         
  //npeaks      30                
  //dge         2               

  std::ifstream fset(fname.Data());
  if(!fset.good()) {
    std::cout<<fname.Data()<<" does not exist!"<<std::endl;
    return false;
  }
  while (!fset.eof()){
      string s,s1,s2,s3;
      stringstream ss,ss1;
      getline(fset,s);  //提取一行
      ss<<s;
      getline(ss,s1,'#'); //提取#之前的内容到s
      ss1<<s1;
      ss1>>s2;
      if (s2=="rootfile") ss1>>st.rootfile;
      if (s2=="ncanvas") ss1>>st.ncanvas;
      if (s2=="xmin")   ss1>>st.xmin;
      if (s2=="xmax")   ss1>>st.xmax;
      if (s2=="npeaks")  ss1>>st.npeaks;
      if (s2=="dge")    ss1>>st.dge;
      if (s2=="histname")
          while (ss1>>s3) st.histname.push_back(s3);
  }
  return true;
}
void newcanvas(int ncy1, TString sctitle)
{
  ic++;
  ic = ic%5;//st.ncanvas;
  ncy = ncy1;
  double w = 800;
  double h = 800;
  if(ncy1 == 1) h = 400;
  sctitle = Form("%s_canv%d", sctitle.Data(), ic);
  if(TCanvas *cc = (TCanvas*)gROOT->FindObject(Form("canv%d", ic))) {
    tlcanv->Remove(cc);
    delete cc;
  }
  ca[ic] = new TCanvas(Form("canv%i",ic), sctitle.Data(), w, h);

  ca[ic]->Divide(1, ncy);
  for(int i = 1; i <= ncy; i++) {
    ca[ic]->GetPad(i)->SetBottomMargin(0.05);
    ca[ic]->GetPad(i)->SetTopMargin(0.005);
    ca[ic]->GetPad(i)->SetLeftMargin(0.08);
    ca[ic]->GetPad(i)->SetRightMargin(0.05);
    if (OpenCross) ca[ic]->GetPad(i)->AddExec("DynamicCross","DynamicCross(ncy)");
    if (ncy==1) ca[ic]->GetPad(i)->AddExec("MarkBin","MarkBin()");  // debugging, do not call !
    ca[ic]->GetPad(i)->AddExec("AxisSync","AxisSync(ncy)");
    ca[ic]->GetPad(i)->AddExec("BaseLineSync","BaseLineSync(ncy)");
  }
  ca[ic]->ToggleEventStatus();
  tlcanv->Add(ca[ic]);
  ((TRootCanvas *)ca[ic]->GetCanvasImp())->DontCallClose();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//generate histogram name according to prefix and title 
TString GetHistName(TString sprefix)
{

  img = mg.find(sprefix);
  int gid;
  if(img != mg.end()){
      img->second++; 
  }
  else {
    gid = 0;
    mg.insert(make_pair(sprefix, gid));
    img = mg.find(sprefix);
  }
  return Form("%s_%d", (img->first).Data(), img->second);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void tpj(int icy)
{
  if(icy > ncy) icy = ncy;
  ca[ic]->cd(icy);
  TString stitle = Form("Total Projection Sepctrum (%d - %d) keV ", st.xmin, st.xmax);
  TString sname = GetHistName("gtpj");   
  TH1F *h;
  if (GateAxis=="X"){
      h = (TH1F*)hg2xy->ProjectionX(sname.Data(), xmin, xmax);
      stitle = "X Axis " + stitle;
  }
  else if (GateAxis=="Y"){
      h = (TH1F*)hg2xy->ProjectionY(sname.Data(), xmin, xmax);
      stitle = "Y Axis " + stitle;
  }
  else {
      std::cout<<"wrong axis name!"<<std::endl;
      return;
  }
  h->SetTitle(stitle.Data());
  if(!tlhist->FindObject(h)) tlhist->Add(h);
  peaks(h);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
    
//two fold - gated on x axis
TString g(double ge, int icy1, TH2 *hg2xyp)
{
  if(ge <= 0) {
    std::cout<<"wrong parameters!"<<std::endl;
    return "error";
  }
  if(icy1 > ncy) icy = ncy;
  else icy = icy1;
  ca[ic]->cd(icy);
    
  hg2xyp->SetAxisRange(st.xmin, st.xmax, "X");
  hg2xyp->SetAxisRange(st.xmin, st.xmax, "Y");

  TAxis *axis;
  if (GateAxis=="X") axis = hg2xyp->GetXaxis();
  else if (GateAxis=="Y") axis = hg2xyp->GetYaxis();
  else {
      std::cout<<"wrong axis name!"<<std::endl;
      return "error";
  }
  int gea = axis->FindBin(ge-st.dge);
  int geb = axis->FindBin(ge+st.dge);
  if(gea < xmin || geb > xmax) {
    cout<<"gate width is out of range!"<<endl;
    return "wrong";
  }
  TString stitle = Form("Gate: %.1f keV (%.1f - %.1f)", ge, ge-st.dge, ge+st.dge);
  
  TString sname = GetHistName(Form("g%d", int(ge)));
 
  TH1F *ha;
  if (GateAxis=="X"){
      ha = (TH1F*)hg2xyp->ProjectionY(sname.Data(), gea, geb);
      stitle = "X Axis " + stitle;
  }
  else if (GateAxis=="Y"){
      ha = (TH1F*)hg2xyp->ProjectionX(sname.Data(), gea, geb);
      stitle = "Y Axis " + stitle;
  }
  ha->SetTitle(stitle);
  if(!tlhist->FindObject(ha)) tlhist->Add(ha);
  peaks(ha);
  return sname;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void peaks(TH1 *h, Double_t thres)
{
  double x0 = h->GetBinLowEdge(h->GetNbinsX());
  double x1 = h->GetBinLowEdge(0);
  if(st.xmin < x1) st.xmin = x1;
  if(st.xmax > x0) st.xmax = x0;
  h->SetAxisRange(st.xmin, st.xmax, "X");
  double ymin = h->GetBinContent(h->GetMinimumBin());
  double ymax = h->GetBinContent(h->GetMaximumBin())*1.3;
  if (ymin < 0 && ymin < -0.1*ymax) ymin = -0.1*ymax;//ymin=0.01; 
  std::cout<< CYAN << ic << " " << icy << " " << h->GetName() <<"  ["<<h->GetTitle()<<"]"<<RESET<<std::endl;
  h->GetYaxis()->SetNoExponent();
  h->SetLineColor(kBlue);
  h->SetFillColorAlpha(kCyan,0.15);
  TSpectrum *s = new TSpectrum(1000);
  h->Sumw2(0);
  h->SetAxisRange(ymin, ymax, "Y");
  h->SetStats(0);
  Int_t nfound = 0;
  Int_t nloop = 0;
  while(nloop < 1000){
    nfound = s->Search(h, 1.5, "", thres);
    if(nfound > st.npeaks) thres += 0.0001;
    else thres -= 0.0001;
    if(thres < 0 || TMath::Abs(nfound-st.npeaks) < 3 ) break;
    nloop++;
  }
  
  TPolyMarker *pm = (TPolyMarker *)
                      h->GetListOfFunctions()->FindObject("TPolyMarker");
  pm->SetMarkerStyle(2);//32
  pm->SetMarkerColor(kRed);
  pm->SetMarkerSize(0.5);//0.4
  Double_t *xpeaks = s->GetPositionX();
  Double_t *ypeaks = s->GetPositionY();
  //to improve the accuracy of peak positions.
  // for(int i = 0; i < 1; i++) {
    for(int j = 0; j < nfound; j++) {
      int binx0 = h->FindBin(xpeaks[j]);
      if(binx0 > 1 && binx0 < xmax) {
	float ymax = ypeaks[j];
	float ya = h->GetBinContent(binx0-1);
	float yb = h->GetBinContent(binx0+1);
	if(ymax < ya){
	  xpeaks[j] = h->GetBinCenter(binx0-1);
	  ypeaks[j] = ya;
	  pm->SetPoint(j, xpeaks[j], ypeaks[j]);	
	}
	if(ymax < yb){
	  xpeaks[j] = h->GetBinCenter(binx0+1);
	  ypeaks[j] = yb;
	  pm->SetPoint(j, xpeaks[j], ypeaks[j]);    
	}
      }
    }
  // }
  
  if (deploymark){
      // std::map<int, int> mg;
      for(int j = 0 ;j < nfound; j++) {
        std::stringstream ss;
        ss << xpeaks[j];
        if(ypeaks[j] < 3) continue;
        TString s1 = ss.str();
        TLatex *tex = new TLatex(xpeaks[j], ypeaks[j]+ymax*0.02, s1);
        tex->SetTextFont(133);//13
        tex->SetTextSize(16);
        tex->SetTextAlign(12);
        tex->SetTextAngle(90);
        tex->SetTextColor(kRed);
        tex->Draw();
        h->GetListOfFunctions()->Add(tex);
        // mg.insert(make_pair(xpeaks[j], ypeaks[j]));
      }
      pm->Draw();
  }
  
  TLine *l1 = new TLine(st.xmin, 0, st.xmax, 0);
  l1->SetLineColorAlpha(kRed, 0.7);
  l1->Draw();
  // please don't delete l1 !!!!
  
  ca[ic]->Draw();

  delete s;
}

void DynamicCross(int nPad)
{
  // std::cout<<gPad->GetEvent()<<std::endl;
  
  // if(gPad->GetEvent() != 51) return;
  // if(gPad->GetEvent() == kMouseEnter) return;

  if (!gPad->GetSelected()) return;
  gPad->GetCanvas()->FeedbackMode(kTRUE);

  TPad *selectedPad = (TPad*)gPad;
  TCanvas *canvas = (TCanvas*)selectedPad->GetMother();
    
  double x = selectedPad->AbsPixeltoX( selectedPad->GetEventX() );
  double y = selectedPad->AbsPixeltoY( selectedPad->GetEventY() );
    
  for (int i = 1; i <= nPad; i++){
    if (!canvas->GetPad(i)) continue;
    canvas->GetPad(i)->cd();

    // gPad->GetListOfPrimitives()->Print();
    
    int pxold = gPad->GetUniqueID()%10000;
    int pyold = gPad->GetUniqueID()/10000;
    int px = gPad->XtoPixel(x);
    int py = gPad->YtoPixel(y);
       
    int pxmin = gPad->XtoPixel( gPad->GetUxmin() );
    int pxmax = gPad->XtoPixel( gPad->GetUxmax() );
    int pymin = gPad->YtoPixel( gPad->GetUymax() );
    int pymax = gPad->YtoPixel( gPad->GetUymin() );

#ifndef R__HAS_COCOA
    if (pxold) gVirtualX->DrawLine(pxold, pymin, pxold, pymax);
    if (pyold) gVirtualX->DrawLine(pxmin, pyold, pxmax, pyold);
#endif // R__HAS_COCOA
    
    if (px < pxmin || px > pxmax) px = 0;
    if (py < pymin || py > pymax) py = 0;    

#ifndef R__HAS_COCOA    
    if (px) gVirtualX->DrawLine(px, pymin, px, pymax);
    if (py) gVirtualX->DrawLine(pxmin, py, pxmax, py);
#endif // R__HAS_COCOA
    
    gPad->SetUniqueID(py*10000+px);
  }
    
  for (int i = 1; i <= nPad; i++)
    if (canvas->GetPad(i)){
        canvas->GetPad(i)->Update();
    }
  selectedPad->cd();
}

void AxisSync(int nPad)
{
  if (gPad->GetEvent() != 11) return;
  if (!gPad->GetSelected()) return;
  gPad->GetCanvas()->FeedbackMode(kTRUE);
    
  gPad->Update();   //确保坐标轴改变生效
    
  TPad *selectedPad = (TPad*)gPad;
  double uxmin = gPad->GetUxmin();
  double uxmax = gPad->GetUxmax();
    
  TCanvas *canvas = (TCanvas*)selectedPad->GetMother();
  for (int i = 1; i <= nPad; i++){
      if (!canvas->GetPad(i)) continue;
      //if (canvas->GetPad(i) == selectedPad) continue;
      canvas->GetPad(i)->cd();
        
      TH1 *h = nullptr;
      for (const auto&& gObj: *gPad->GetListOfPrimitives())
          if (gObj->InheritsFrom(TH1::Class())){
          h = (TH1*)gObj;
          break;
      }
      if (h == nullptr) continue;

      h->SetAxisRange(uxmin, uxmax, "X");

      gPad->Modified();   //确保及时刷新
      gPad->Update();
  }
  selectedPad->cd();
}

void BaseLineSync(int nPad)
{
  if (gPad->GetEvent() != 11) return;
  if (!gPad->GetSelected()) return;
  gPad->GetCanvas()->FeedbackMode(kTRUE);
    
  TPad *selectedPad = (TPad*)gPad;
  TCanvas *canvas = (TCanvas*)selectedPad->GetMother();
    
  for (int i = 1; i <= nPad; i++){
      if (!canvas->GetPad(i)) continue;
      canvas->GetPad(i)->cd();
        
      double uxmin = gPad->GetUxmin();
      double uxmax = gPad->GetUxmax();

      TLine *l1 = (TLine *)gPad->GetListOfPrimitives()->FindObject("TLine");
      if(l1 != nullptr){
          l1->SetX1(uxmin);
          l1->SetX2(uxmax);
      }
      gPad->Modified();   //确保及时刷新
      gPad->Update();
  }
  selectedPad->cd();
}

// debugging ......
void MarkBin()
{
    if (gPad->GetEvent()!=11) return;
    if (!gPad->GetSelected()) return;
    gPad->GetCanvas()->FeedbackMode(kTRUE);  
    
    int pxmin = gPad->XtoPixel( gPad->GetUxmin() );
    int pxmax = gPad->XtoPixel( gPad->GetUxmax() );
    int pymin = gPad->YtoPixel( gPad->GetUymax() );
    int pymax = gPad->YtoPixel( gPad->GetUymin() );
    int px = gPad->GetEventX();
    int py = gPad->GetEventY();
    if (px<pxmin || px>pxmax || py<pymin || py>pymax) return;
    
    TH1 *h;
    for (const auto&& gObj: *gPad->GetListOfPrimitives())
        if (gObj->InheritsFrom(TH1::Class())){
            h = (TH1*)gObj;
            break;
        }
    
    Float_t xx = gPad->AbsPixeltoX(px);
    int cbin = h->FindBin(xx);
   
    int xbin = cbin;
    // for (int ibin=cbin-1;ibin<=cbin+1;ibin++)
        // if ( h->GetBinContent(ibin) > h->GetBinContent(xbin) )  xbin = ibin;
    int y = h->GetBinContent(xbin);
    
    // if click repeatedly, erase TLatex and TMarker
    bool found = false;
    for (const auto&& gObj: *h->GetListOfFunctions()){
        found = gObj->InheritsFrom(TLatex::Class()) && xbin==gObj->GetUniqueID();
        if (found) {
            h->GetListOfFunctions()->Remove(gObj);
            break;
        }
    }
    for (const auto&& gObj: *h->GetListOfFunctions())
        if (gObj->InheritsFrom(TMarker::Class()) && xbin==gObj->GetUniqueID()) {
            h->GetListOfFunctions()->Remove(gObj);
            break;
        }
    
    // set new TLatex and TMarker
    if (!found){
        int xtex = (h->GetXaxis()->GetBinLowEdge(xbin)+h->GetXaxis()->GetBinUpEdge(xbin))/2.;
        TString s = Form("%d",xtex);
        float ymax = h->GetBinContent(h->GetMaximumBin())*1.5;
        TLatex *tex=new TLatex(xtex,y+ymax*0.02,s);
        tex->SetUniqueID(xbin);
        tex->SetTextFont(133);//13
        tex->SetTextSize(30);
        tex->SetTextAlign(12);
        tex->SetTextAngle(90);
        tex->SetTextColor(kRed);
        h->GetListOfFunctions()->Add(tex);

        TMarker *mark = new TMarker(xtex,y,2);
        mark->SetUniqueID(xbin);
        mark->SetMarkerColor(kRed);
        mark->SetMarkerSize(0.5);//0.4
        h->GetListOfFunctions()->Add(mark);
    }
    //gPad->Modified();//确保及时刷新
    //gPad->Update();
}
 
