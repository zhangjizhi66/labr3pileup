// UserDefine.h --- 
// 

#ifndef _USERDEFINE_H_
#define _USERDEFINE_H_

#include <vector>

//#define CALIBRATION

#define FILEPATH "../../data_release/"   //Path to the original and generated ROOT file

#define EVENTFILENAME "event"
#define CHECKFILENAME "check"
#define STANDARDWAVEFILENAME "standardwave"
#define ANAFILENAME "ana"
#define CALFILENAME "ecal"
#define MATFILENAME "mat"

//#define USE_CURRENT_STDWAVE
#define USE_FIXED_STDWAVE 95

static const int ndet = 16;
static const int nid = 16;

bool valid(int det, int id);

static const int L = 3;               // fastfilter parameter
static const int trise = 10;            // wave rise time
static const double fastfiltertrigger = 5;   // times of noise level

static const int maxoverflow = 20000;
static const int tdc2ns = 2;           // 2ns / point

static const int length = 100;   // standard waveform length = 2*length

#ifdef CALIBRATION
    static const int nbase = 400;     // baseline length at the head of waveform
#else
    static const int nbase = 1000;     // baseline length at the head of waveform
#endif

static const double standardfwhm = length/5;
static const int nfitpnt = 2*length;

////////////////////////////////////////////////////1GetStandardWave////////////////////////////////////////////////////////

static const int minheight = 1000;    // minimum height to build standard wave
static const int maxheight = maxoverflow;  // maximum height to build standard wave

static const int refbin = 10000;  // balance statistics of each bin to the bin at height = refbin

////////////////////////////////////////////////////1GetStandardWave////////////////////////////////////////////////////////

///////////////////////////////////////////////////////2FitWave/////////////////////////////////////////////////////////////

static const int rangeuseleft = -20;  // standard waveform range use
static const int rangeuseright = 80;  // standard waveform range use

static const int minpileup = rangeuseright - rangeuseleft;  // pile up peaks will be fitted together
static const int peak2trig = trise;

static const double overflow = 10000;  // find overflow value at (overflow, +inf)

// fitting parameter
static const double maxpeakfitinterval = 0.1;      // max peak initial value interval
static const int maxaddpeaks = 100;             // max npeaks allowed being added to fitting
static const int lowestfittedpeak = 20;       // if lower, abandon this peak
static const double optimizechi2ndf = 0.7;  // add a peak must lower chi2ndf by this portion, otherwise no extra peak
static const int lowestheight = 3;           // lowest height, times of noise level

///////////////////////////////////////////////////////2FitWave/////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////Calibrate////////////////////////////////////////////////////////////

static const int parameterprecision = 6;

static const int flushlowedge = 4000;

#ifdef CALIBRATION
    static std::vector<double> ve{661.66};
    static std::vector<double> vchleft{2700};
    static std::vector<double> vchright{3000};
#else
    static std::vector<double> ve{165.8, 217.5};
    static std::vector<double> vchleft{650, 900};
    static std::vector<double> vchright{900, 1100};
#endif

static const double minfitrange = 0;
static const double maxfitrange = maxoverflow;
static const int nfitbins = (maxfitrange - minfitrange)/5;

///////////////////////////////////////////////////////Calibrate////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////Matrix//////////////////////////////////////////////////////////////

static const double tmin = -20000;
static const double tmax = 20000;
static const int tbins = 4000;

static const int matrixwindow = 20;  // point

static const double emin = 0;
static const double emax = 4000;
static const int ebins = 2000;

////////////////////////////////////////////////////////Matrix//////////////////////////////////////////////////////////////

#define WAVEFORM

#define EVENTTIMEWINDOWSWIDTH  1000   //ns

#endif /* _USERDEFINE_H_ */

// 
// UserDefine.h ends here
