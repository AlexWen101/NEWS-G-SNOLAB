#include "TROOT.h"
#include <TMath.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH3.h>
#include <TH3F.h>
#include <TH3D.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TTree.h>
#include <TFile.h>
#include <TF1.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TPaveLabel.h>
#include <TPaveText.h>
#include <TGraphErrors.h>
#include <TRandom.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <numeric>
#include <cmath>

#include <stdio.h>
#include <stdlib.h>

#include "Riostream.h"
#include "TString.h"
#include "TSystem.h"

#include "TMath.h"
#include <stdbool.h>
#include <locale.h>



//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//
// This macro takes the output of the Geant4 simulation "ShieldSimulation" and processes them so 
// they may be written to ROOT files and appear on a Amplitude vs. Rise Time graph. This will help give 
// an idea of differentiating between the background and the elastic neutron collision that result from the neutron 
// calibration source. 
//
// Before calling any functions, start root and compile the macro: 
// $ root
// $ .L NeutronSimulationMacro.cc+
//
//  If some of the functionality of this macro becomes obsolete
//      you can also just use the single function that you happen to need. 
//
// USING THE MACRO: 
// There are three functions: Primary, Secondary, and HistogramFiller. 
// 1. Primary is the function that sorts every event into order of ascending global time. It will take the longest to complete
//      since it uses large vectors and a sorting algorithm on each one (probably could be optimized more!). It eats a .txt input 
//      file (converted from the .csv output from the simulation) and spits out a re-ordered .txt file. 
//      To use, call Primary("<name of your .txt input file>","<name of the output file>")
//
// 2. Secondary is the main processing function where the ordered steps are read in event-by-event, information is extracted
//      with various distrbutions, written to a temporary histogram, and writes the RT vs. Energy deposit information
//      to a new file. This function also analyzes some of the secondaries and rudimentarily classifies each event as a 
//      neutron elastic, inelastic, or whatever else you want it to. It writes the classified event type to a new variable called 
//      eventType. You can find more details about all the processing that goes on in the comments in the function. Eats the 
//      output from Primary() and spits out another txt file.
//      To use, call Secondary("<name of input file>","<name of output file>")
//
// 3. HistogramFiller is the function that creates the ROOT 1D and 2D histograms. It simply sorts events to different histograms
//      based on various characteristics of that event. It's also where you can adjust the aesthetics of the histograms
//      and any other preferences like binning, sorting, etc. It eats the output file from Secondary() and spits out a .root output file.
//      Also writes and fills a tree with all the data. 
//      To use, call HistogramFiller("<name of input file>","<name of your .root output file>")
//
// Alex Wen (Summer 2018) - the processing methodology used in Secondary() is adapted from Alexis Brossard's work 
//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//



//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//
// Declarations - mostly used in Primary()
//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//
using namespace std;

string line(""); 
int nline(0); 

// Struct used to store temporary information to be re-ordered based on global time.
// The processing can be sped up considerably by trimming unecessary data from being printed and read in here. 
struct data_struct {
    public:
    int eventNb;
    double globalTime; 
    TString particleName; 
    TString particleType; 
    double initialEkin; 
    TString processCreator; 
    int particleNb; 
    TString motherName; 
    int motherNb; 
    //vector<double> positionPre{3}; 
    //vector<double> positionPost{3};
    double stepEnergyDifference; 
    double stepTotalEnergy; 
    double stepKineticEnergy;
    TString process; 
    //vector<double> positionInitial{3};
    TString refPartName; 
    //vector<double> positionRef{3};
    TString timeParticle; 
    //vector<double> positionTime{3}; 
    int subEventNb;
    double positionPreX, positionPreY, positionPreZ;
        double positionPostX, positionPostY, positionPostZ; 
        double positionInitialX, positionInitialY, positionInitialZ;
        double positionRefX, positionRefY, positionRefZ; 
        double positionTimeX, positionTimeY, positionTimeZ;
} data_current;

int subSubEventNb_int; 
// Big vector used to contain the structs
vector<data_struct> data_vector; 

// Simple swapping function used in the later sorting function
void swap_entry(vector<data_struct> &vector, int x, int y) {
    data_struct temp = vector[x];
    vector[x] = vector[y];
    vector[y] = temp;
}

// Sorting algorithm (Quicksort) to operate on the data_struct vector to put it in order of global time. 
void quicksort(vector<data_struct> &vector, int l, int r) {
    
    if (l>=r) {
        return;
    }

    double pivot = vector[r].globalTime;

    int cnt = l; 

    for (int i = l; i <= r; i++) {
        if (vector[i].globalTime <= pivot) {
            swap_entry(vector, cnt, i);
            cnt++; 
        }
    }

    quicksort(vector, l, cnt-2);
    quicksort(vector, cnt, r); 
}

int eventNb_pre = 0; 
double globalTime_pre(0.); 

//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//
// Declarations - Mostly used in Secondary()
//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//

int eventNb;
double globalTime; 
TString particleName; 
TString particleType; 
double initialEkin; 
TString processCreator; 
int particleNb; 
TString motherName; 
int motherNb; 
//vector<double> positionPre(3,0); 
//vector<double> positionPost(3,0);
double stepEnergyDifference; 
double stepTotalEnergy; 
double stepKineticEnergy;
TString process; 
//vector<double> positionInitial(3,0);
TString refPartName; 
//vector<double> positionRef(3,0);
TString timeParticle; 
//vector<double> positionTime(3,0); 
int subSubEventNb; 
double positionPreX, positionPreY, positionPreZ;
double positionPostX, positionPostY, positionPostZ; 
double positionInitialX, positionInitialY, positionInitialZ;
double positionRefX, positionRefY, positionRefZ; 
double positionTimeX, positionTimeY, positionTimeZ;

// Eventtype variable. If the type can't be classified as anything it is "other" by default. 
TString eventType("other"); 

struct pulsedata_struct {
    public:
    TString particleName_print; 
    TString processCreator_print; 
    int particleNb_print; 
    int motherNb_print;  
} pulsedata;

vector<pulsedata_struct> pulsedata_vector; 
vector<int> particleNb_vector; 
vector<TString> secondaries_vector; 
vector<TString> secondaries_print_vector; 
vector<double> radius_vector; 
// other variables 

double radius_sum(0); 
double radius_average(0); 

double radius_stdev(0); 

// selecting events /counters

int subSubEventNb_pre(0);
//int eventNb_pre(0);

double 	eventEnergy(0.);
double	stepTime();



//Variables related to precessed step
double radial_distance(0);
double primary_elec(0);
double secondary_elec(0);
double drift_time(0);
int eventNb_processed(0);

// variables required for integral

int k(1);
double sumPrimary(0);
Float_t sumSecondary(0); 
double sum(0); 
double t10(0);
double t90(0); 
double maxhisto(0); 
double riseTime(0); 

// variables to print

TString timeParticleName_pre;
Float_t timeParticleRadius_pre;
TString firstParticleName_pre;
Float_t firstParticleInitEkin_pre;
TString process_pre;
Float_t edepEvent; 


TString secondaries;
TString secondaries_print; 
// Reading in data from magboltz

// This is is reading information form another ROOT file containing graphs generated from Magboltz (produced by Alexis)
// These graphs indicate the drift time as a function of radius and its variance
TFile* fmag=new TFile("News_Ne_4000V_v5.root","READ");
TGraph * DT=(TGraph*)fmag->Get("DriftT");  
TGraph * SigmaDT=(TGraph*)fmag->Get("SigmaDriftTime");

// Create the histogram that you write to in every event and obtain the rise time from 
TH1F *histoPulse = new TH1F("histoPulse","Pulse", 4000, 0, 4000); 
//TH1F *histoPulseInt = new TH1F("histoPulseInt","Pulse", 4000, 0, 4000); //Histo to show on pulse
//TH1F *histoSpectre = new TH1F("histoSpectre","Pulse", 2000, 0, 1000000); //Histo to show on pulse

//TH2F *histoPrimRT = new TH2F("histoPrimRT","Pulse", 20000, 0, 1000000, 500, 0, 500); //Useful to check 
//TCanvas *c1 = new TCanvas("c1","D",0,0,1300,1000);
//TCanvas *c3 = new TCanvas("c3","Pulse",0,0,1300,1000);


double Polya(double *palpha, double *par) {
    double th = par[0];
    double meangain = par[1];
    double nrat = palpha[0];
    nrat = nrat/meangain;
    double result = (TMath::Power((1.+th),(1.+th))/TMath::Gamma(1.+th))*TMath::Power(nrat,th)*TMath::Exp(-(1.+th)*nrat);
    return result/meangain;
}

double thetapolya=0.3;
double meangainpolya=36;

// class to accurately compute variance and stdev. 
// Shamelessly copied verbatim from https://www.johndcook.com/blog/standard_deviation/
class RunningStat {
    public:
        RunningStat() : m_n(0) {}

        void Clear()
        {
            m_n = 0;
        }

        void Push(double x)
        {
            m_n++;

            // See Knuth TAOCP vol 2, 3rd edition, page 232
            if (m_n == 1)
            {
                m_oldM = m_newM = x;
                m_oldS = 0.0;
            }
            else
            {
                m_newM = m_oldM + (x - m_oldM)/m_n;
                m_newS = m_oldS + (x - m_oldM)*(x - m_newM);
    
                // set up for next iteration
                m_oldM = m_newM; 
                m_oldS = m_newS;
            }
        }

        int NumDataValues() const
        {
            return m_n;
        }

        double Mean() const
        {
            return (m_n > 0) ? m_newM : 0.0;
        }

        double Variance() const
        {
            return ( (m_n > 1) ? m_newS/(m_n - 1) : 0.0 );
        }

        double StandardDeviation() const
        {
            return sqrt( Variance() );
        }

    private:
        int m_n;
        double m_oldM, m_newM, m_oldS, m_newS;
} rs;



//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//
// Declarations - mostly used in HistogramFiller()
//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//

// Variables to read and control the reading of file
//string line("");
//int nline(0);
// Variables who take value from output_step.txt

// New variables to be printed 
TString TimeParticleName_prev ;
double TimeParticleRadius_prev ;
double FirstParticleInitEkin_prev ;
double RFStep_prev ;
TString Process_prev ;
double EdepEvent ; 
double risetime;
TString FirstProcess_prev;
TString firstParticleName;
//int eventNb; 
//int subSubEventNb; 
//string eventType; 
//string secondaries_print; 

struct TStringClass_struct {
    public: 
    TString firstParticleName;
    TString eventType_histo; 
} TStringClass; 

double m_SumPrimary(0);
double m_SumSecondaryEV(0); //Energy in eV
double m_SumSecondary(0);   //Energy in keV
//double risetime(0);

// constants that control the binning and range of the histograms:
// Emax is the maximum energy (x-axis)
// RTmax is the maximum Rise time (y-axis)
// the binDivisors are how much the maxima are divided by; the quotient is the number of bins you have on each axis. 
// Note: for 1D histograms only the energy is relevant. 
double Emax(300);
double RTmax(1000);
double binDivisorX(1);
double binDivisorY(1);

int firstParticleName_ref(0); 
int eventType_ref(0);




//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//
// Primary()
//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//

int Primary(string inputfile, string outputfile) {

    // Inputfile: user inputted name as the sold argument of the function
    ifstream infile(inputfile,std::ifstream::in);
    // Outputfile: automatically named (the same name that Primary Processing reads)
    ofstream outfile(outputfile,std::ifstream::trunc);

    while(!infile.eof()) {

        // Iterate over every line in the input file
        nline++; 

        // Displays the progress of the line reading
        if (nline%20000==0) {
            cout << "Working on line " << nline << "!" << endl;  
        }


        // All input file data is read into variables of the structure data_current
        infile >>
        data_current.eventNb >>
        data_current.globalTime >>
        data_current.particleName >>
        data_current.particleType >> 
        data_current.initialEkin >>
        data_current.processCreator >>
        data_current.particleNb >>
        data_current.motherName >>
        data_current.motherNb >>
        data_current.positionPreX >>
        data_current.positionPreY >>
        data_current.positionPreZ >>
        data_current.positionPostX >>
        data_current.positionPostY >>
        data_current.positionPostZ >>
        data_current.stepEnergyDifference >>
        data_current.stepTotalEnergy >>
        data_current.stepKineticEnergy >>
        data_current.process >>
        data_current.positionInitialX >>
        data_current.positionInitialY >>
        data_current.positionInitialZ >>
        data_current.refPartName >>
        data_current.positionRefX >>
        data_current.positionRefY >>
        data_current.positionRefZ >>
        data_current.timeParticle >>
        data_current.positionTimeX >>
        data_current.positionTimeY >>
        data_current.positionTimeZ >> 
        data_current.subEventNb; 



        //cout << data_current.positionPost[2] << " at " << nline << endl; 
        

        if (eventNb_pre == data_current.eventNb) {

            data_vector.push_back(data_current); 
             
            //cout << "The normal data current is " << data_current.positionTime[2] << endl;

        }
        
        else if (eventNb_pre != data_current.eventNb) {
            
            
            //cout << "Handling event # " <<data_current.eventNb<< "!" <<endl;
            subSubEventNb_int = 0;
            
            
            //subSubEventNb.push_back(0); 
            
            int size = data_vector.size(); 

            // Quick sorting to sort the vectors in order of ascending global time
            // Sorted based on the global time, but all changes in order are also done exactly with the other values
            // (See Quick sort algorithm)
        

            quicksort(data_vector, 0, size-1); 

             /*
            for (int k = 0; k < size-1; k++) {

                if ((data_vector[k+1].globalTime - data_vector[k].globalTime) > 0.001)  {

                    subSubEventNb_int++ ;

                }
                
                subSubEventNb.push_back(subSubEventNb_int);  
        }
                */
            
            

            for (int k = 0; k < size; k++) {

                if (0 < k < size) {
                    if ((data_vector[k].globalTime - data_vector[k-1].globalTime) > 0.001)  {

                        subSubEventNb_int++;

                    }
                }

                

                outfile <<" "<<
                data_vector[k].eventNb <<" "<<
                data_vector[k].globalTime <<" "<<
                data_vector[k].particleName <<" "<<
                //data_vector[k].particleType <<" "<< 
                data_vector[k].initialEkin <<" "<<
                data_vector[k].processCreator <<" "<<
                data_vector[k].particleNb <<" "<<
                data_vector[k].motherName <<" "<<
                data_vector[k].motherNb <<" "<<
                //data_vector[k].positionPre[0] <<" "<<
                //data_vector[k].positionPre[1] <<" "<<
                //data_vector[k].positionPre[2] <<" "<<
                data_vector[k].positionPostX <<" "<<
                data_vector[k].positionPostY <<" "<<
                data_vector[k].positionPostZ <<" "<<
                data_vector[k].stepEnergyDifference <<" "<<
                data_vector[k].stepTotalEnergy <<" "<<
                data_vector[k].stepKineticEnergy <<" "<<
                data_vector[k].process <<" "<<
                //data_vector[k].positionInitial[0] <<" "<<
                //data_vector[k].positionInitial[1] <<" "<<
                //data_vector[k].positionInitial[2] <<" "<<
                //data_vector[k].refPartName <<" "<<
                //data_vector[k].positionRef[0] <<" "<<
                //data_vector[k].positionRef[1] <<" "<<
                //data_vector[k].positionRef[2] <<" "<<
                data_vector[k].timeParticle <<" "<<
                //data_vector[k].positionTimeX <<" "<<
                //data_vector[k].positionTimeY <<" "<<
                //data_vector[k].positionTimeZ <<" "<<
                //data_vector[k].subEventNb <<" "<<
                subSubEventNb_int << endl; 
            
            }

            data_vector.clear();

            data_vector.push_back(data_current); 

        }

        eventNb_pre = data_current.eventNb;

        globalTime_pre = data_current.globalTime; 

    }

return 0;

}


//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//
// Secondary()
//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//

int Secondary(const char* inputfile, const char * outputfile) {

    ifstream data(inputfile,ios::in);
    ofstream event(outputfile,ios::trunc);

   

        TF1 *f_Polya = new TF1("Polya",Polya,0,10000,2);
        f_Polya->SetNpx(10000);
        f_Polya->SetParameter(0,thetapolya);
        f_Polya->SetParameter(1,meangainpolya);

        TCanvas*canvas1 = new TCanvas("canvas1","canvas1",1500,1200); 
       // TCanvas*canvas2 = new TCanvas("canvas2","canvas2",1500,500);

        TCanvas*main_canvas = new TCanvas("main","main",1500,1300); 
        main_canvas->Divide(1,2); 
        

    while(getline(data,line)) {

        nline++; 
        if (nline%10000==0) {cout << "Reading of line " <<nline<<endl;}

        data >>
        eventNb >>
        globalTime >>
        particleName >>
        //particleType >> 
        initialEkin >>
        processCreator >>
        particleNb >>
        motherName >>
        motherNb >>
        //positionPre[0] >>
        //positionPre[1] >>
        //positionPre[2] >>
        positionPostX >>
        positionPostY >>
        positionPostZ >>
        stepEnergyDifference >>
        stepTotalEnergy >>
        stepKineticEnergy >>
        process >>
        //positionInitial[0] >>
        //positionInitial[1] >>
        //positionInitial[2] >>
        //refPartName >>
        //positionRef[0] >>
        //positionRef[1] >>
        //positionRef[2] >>
        timeParticle >>
        //positionTime[0] >>
        //positionTime[1] >>
        //positionTime[2] >> 
        subSubEventNb;

        pulsedata.particleName_print = particleName; 
        pulsedata.particleNb_print = particleNb; 
        pulsedata.processCreator_print = processCreator; 
        pulsedata.motherNb_print = motherNb; 

        //cout << eventNb <<" "<< subSubEventNb << endl;

        //printf("%i\n",eventNb);

        radial_distance = TMath::Sqrt(TMath::Power(positionPostX,2)+TMath::Power(positionPostY,2)+TMath::Power(positionPostZ,2));
        primary_elec = gRandom->Poisson((stepEnergyDifference*1.e6)/(36.));
        drift_time = DT->Eval(radial_distance/10.);

         

        //cout <<"Drift time is "<< drift_time << endl;

        //printf("%f\n",radial_distance);

        if ((eventNb == eventNb_pre) && (subSubEventNb == subSubEventNb_pre)) {
            for (int j = 0; j < primary_elec; j++) {
                secondary_elec = f_Polya->GetRandom();
                sumSecondary += secondary_elec; 
                histoPulse->Fill(drift_time + gRandom->Gaus(0,SigmaDT->Eval(radial_distance/10.)),secondary_elec);
            }

            pulsedata_vector.push_back(pulsedata);
            particleNb_vector.push_back(particleNb); 

            radius_vector.push_back(radial_distance); 

            //cout << secondary_elec << endl;

            sumPrimary += primary_elec; 
            edepEvent += stepEnergyDifference; 
        }


            

        else if (!pulsedata_vector.empty()) {
       
           
            sum = 0;
            
            if (( eventNb_pre == 53735) && (subSubEventNb_pre == 0) ) {
                main_canvas->cd(1); 
                gStyle->SetOptStat(""); 
                //gStyle->SetOptTitle(0); 
                TH1F * histoPulse_raw = (TH1F*)histoPulse->Clone(); 
                histoPulse_raw->SetTitle("Raw Pulse");

                histoPulse_raw->GetXaxis()->SetRangeUser(1000,1600);
                histoPulse_raw->GetXaxis()->SetLabelOffset(999);
                histoPulse_raw->GetXaxis()->SetLabelSize(0);

                histoPulse_raw->GetYaxis()->SetTitle("ADU (#propto # of electrons)");
                histoPulse_raw->GetYaxis()->SetTitleSize();
                histoPulse_raw->Draw("hist");  
                

            }
            //if (eventNb_processed == 10) {c3->cd(1); histoPulse->DrawCopy();}

            for (int j=1; j<=histoPulse->GetNbinsX(); j++) {
                sum +=histoPulse->GetBinContent(j); 
                histoPulse->SetBinContent(j,sum);
            }

            if (( eventNb_pre == 53735) && (subSubEventNb_pre == 0) ) {
                main_canvas->cd(2); 
                TH1F * histoPulse_int = (TH1F*)histoPulse->Clone();
                histoPulse_int->SetTitle("Integrated Pulse");

                histoPulse_int->GetXaxis()->SetRangeUser(1000,1600);
                histoPulse_int->GetXaxis()->SetTitle("Time (#mus)"); 
                histoPulse_int->GetXaxis()->SetTitleSize();

                histoPulse_int->GetYaxis()->SetTitle("ADU (#propto # of electrons)");
                histoPulse_int->GetYaxis()->SetTitleSize();
                histoPulse_int->Draw("hist"); 

                canvas1->cd(); 
                histoPulse_int->Draw("hist"); 
            }

            //if (eventNb_processed == 10) {c3->cd(2); histoPulse->DrawCopy();}

            t10 = 0.;
            t90 = 0.;
            maxhisto = histoPulse->GetMaximum();

            //cout <<"Max histo "<< maxhisto << endl; 

            for (int j=1; j<=histoPulse->GetNbinsX(); j++) {
                if ((histoPulse->GetBinContent(j)<(maxhisto/10.)) && (histoPulse->GetBinContent(j+1)>(maxhisto/10.))) {
                    t10 = histoPulse->GetBinCenter(j);
                }

                //cout << "t10 inside is " << t10 << endl;

                if ((histoPulse->GetBinContent(j)<(7.5*maxhisto/10.)) && (histoPulse->GetBinContent(j+1)>(7.5*maxhisto/10.))) {
                    t90 = histoPulse->GetBinCenter(j);
                }
            }
            
            
            vector<int>::iterator result = min_element(begin(particleNb_vector), end(particleNb_vector));
            int min_element_index = distance(begin(particleNb_vector), result); 


            //if (sumSecondary>0.001) {
                //eventNb_processed++; 
                riseTime = t90 - t10; 

               // cout << "rise time is " << riseTime << endl; 

                //histoPrimRT->Fill(sumSecondary,riseTime);
                //histoSpectre->Fill(sumSecondary); 


                

                for (int h = 0 ; h < (int)pulsedata_vector.size(); h++) {
                    
                    rs.Push(radius_vector[h]); 

                    if (pulsedata_vector[h].motherNb_print == pulsedata_vector[min_element_index].particleNb_print) {
                        
                        secondaries_vector.push_back(pulsedata_vector[h].particleName_print); 

                        if (secondaries_print_vector.size() < 6) {
                            secondaries_print_vector.push_back("(" + pulsedata_vector[h].particleName_print+"["+ pulsedata_vector[h].processCreator_print +"]" + ")");
                        }
                    }
                }

                radius_average = rs.Mean(); 
                radius_stdev = rs.StandardDeviation(); 
                
                secondaries_print = accumulate(secondaries_print_vector.begin(), secondaries_print_vector.end(), secondaries_print); 

                if (find(secondaries_vector.begin(), secondaries_vector.end(), "Ne20") != secondaries_vector.end()) {
                    eventType = "neutronElastic"; 
                }
                else if ((find(secondaries_vector.begin(), secondaries_vector.end(), "proton") != secondaries_vector.end()) &&
                            (find(secondaries_vector.begin(), secondaries_vector.end(), "F20") != secondaries_vector.end()) &&
                            (find(secondaries_vector.begin(), secondaries_vector.end(), "gamma") != secondaries_vector.end()) ) {
                                eventType = "neutronInelasticNeDecay";
                            }

                event <<" "<<
                eventNb_pre <<" "<<
                subSubEventNb_pre <<" "<<
                pulsedata_vector[min_element_index].particleName_print <<" "<<
                //pulsedata_vector.size() <<" "<< 
                //pulsedata_vector[min_element_index].processCreator_print <<" "<<
                //timeParticleName_pre <<" "<<
                //timeParticleRadius_pre<<" "<<
                //firstParticleName_pre<<" "<<
                //firstParticleInitEkin_pre<<" "<<
                //process_pre<<" "<<
                edepEvent <<" "<<
                sumPrimary <<" "<<
                sumSecondary <<" "<<
                riseTime <<" "<<
                eventType <<" "<<
                maxhisto <<" "<< // this is the amplitude (of the integrated pulse)
                radius_average <<" "<<
                radius_stdev << endl;//<<" "<<
                //secondaries_print << endl; 

                secondaries_vector.clear();
                secondaries_print_vector.clear(); 
                secondaries = "";  
                secondaries_print = "";  
 
                rs.Clear(); 

                eventType = "other"; 
                
            //}

            histoPulse->Reset(); 

            //if (nline == 100) {c1->cd();
            //    histoPulseInt->Draw();} 
            //histoPulseInt->Reset();

            timeParticleName_pre = timeParticle; 
            //timeParticleRadius_pre = TMath::Sqrt(TMath::Power(positionTime[0],2)+TMath::Power(positionTime[1],2)+TMath::Power(positionTime[2],2));
            firstParticleName_pre = particleName;
            firstParticleInitEkin_pre = initialEkin; 
            //eventNb_pre = eventNb; 
            //subSubEventNb_pre = subSubEventNb; 
            process_pre = process; 
            sumPrimary = 0;
            sumSecondary = 0; 
            edepEvent = 0;


            for (int j=0; j<primary_elec;j++)
			{
				secondary_elec = f_Polya->GetRandom();
				sumSecondary += secondary_elec;
				histoPulse->Fill(drift_time + gRandom->Gaus(0,SigmaDT->Eval(radial_distance/10.)),secondary_elec);
			}
			sumPrimary+=primary_elec;
			edepEvent+=stepEnergyDifference;

            pulsedata_vector.clear(); 
            particleNb_vector.clear(); 
            radius_vector.clear(); 

            pulsedata_vector.push_back(pulsedata); 
            particleNb_vector.push_back(particleNb); 
            radius_vector.push_back(radial_distance); 

        }

        subSubEventNb_pre = subSubEventNb; 
        eventNb_pre = eventNb;

    }

    return 0;

}



//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//
// HistogramFiller()
//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//

int HistogramFiller(const char* inputfile, const char * outputfile) {

	// Create all the histograms to be filled

		// 1-D Spectra
		// General

TH1F *histoSpectrum = new TH1F("Spectrum","Spectrum", int(Emax/binDivisorX), 0, Emax);  

TH1F *histoSpectrumRT32 = new TH1F("Spectrum (RT < 32)","Risetime > 32 Spectrum", int(Emax/binDivisorX), 0, Emax);  
// Gamma and Subatomics
TH1F *histoSpectrumGamma = new TH1F("Gamma_Spectrum","Gamma Spectrum", int(Emax/binDivisorX), 0, Emax);  
TH1F *histoSpectrumElec = new TH1F("Electron_Spectrum","Electron Spectrum", int(Emax/binDivisorX), 0, Emax); 
TH1F *histoSpectrumPosi = new TH1F("Positron_Spectrum","Positron Spectrum", int(Emax/binDivisorX), 0, Emax); 
TH1F *histoSpectrumAlpha = new TH1F("Alpha_Spectrum","Alpha Spectrum", int(Emax/binDivisorX), 0, Emax); 
TH1F *histoSpectrumNeutron = new TH1F("Neutron_Spectrum","Neutron Spectrum", int(Emax/binDivisorX), 0, Emax);
// Elements (more general)
TH1F *histoSpectrumC = new TH1F("C_Spectrum","C Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumO = new TH1F("O_Spectrum","O Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumF = new TH1F("F_Spectrum","F Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumNe = new TH1F("Ne_Spectrum","Ne Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumNa = new TH1F("Na_Spectrum","Na Spectrum", int(Emax/binDivisorX), 0, Emax);
// Atomic Isotopes
TH1F *histoSpectrumO19 = new TH1F("O19_Spectrum","O19 Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumF20 = new TH1F("F20_Spectrum","F20 Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumF21 = new TH1F("F21_Spectrum","F21 Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumF22 = new TH1F("F22_Spectrum","F22 Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumNe20 = new TH1F("Ne20_Spectrum","Ne20 Spectrum", int(Emax/binDivisorX), 0, Emax);
// Other

TH1F *histoSpectrumNeutronElastic = new TH1F("Neutron_Elastic_Spectrum","Neutron Elastic Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumNeutronInelastic = new TH1F("Neutron_Inelastic_Spectrum","Neutron Inelastic Spectrum", int(Emax/binDivisorX), 0, Emax);
TH1F *histoSpectrumOther = new TH1F("Others_Spectrum","Others Spectrum", int(Emax/binDivisorX), 0, Emax);


//---------------------------------------------------------------------------------//
// 2-D Spectra
// General
TH2F *histoSpectrum_2D = new TH2F("RT-Energy","Risetime vs. Energy", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
//TH2F *histoSpectrum_2DColor = new TH2F("RT-Energy C","Risetime vs. Energy, Color Sorted", int(Emax/binDivisorX), 0, 100000 ,int (RTmax), 0., RTmax );
// Gamma and Subatomics

TH2F *histoSpectrumGamma_2D = new TH2F("Gamma_RT-Energy","Risetime vs. Energy (Gammas)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
TH2F *histoSpectrumElec_2D = new TH2F("Electron_RT-Energy","Risetime vs. Energy (Electrons)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
TH2F *histoSpectrumPosi_2D = new TH2F("Positron_RT-Energy","Risetime vs. Energy (Positrons)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
TH2F *histoSpectrumAlpha_2D = new TH2F("Alpha_RT-Energy","Risetime vs. Energy (Alphas)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
TH2F *histoSpectrumNeutron_2D = new TH2F("Neutron_RT-Energy","Risetime vs. Energy (Neutrons)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
//Elements 
TH2F *histoSpectrumC_2D = new TH2F("C_RT-Energy","C Risetime vs. Energy (C)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);
TH2F *histoSpectrumO_2D = new TH2F("O_RT-Energy","O Risetime vs. Energy (O)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);
TH2F *histoSpectrumF_2D = new TH2F("F_RT-Energy","F Risetime vs. Energy (F)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);
TH2F *histoSpectrumNe_2D = new TH2F("Ne_RT-Energy","Ne Risetime vs. Energy (Ne)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);
TH2F *histoSpectrumNa_2D = new TH2F("Na_RT-Energy","Na Risetime vs. Energy (Na)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);
// Atomic Isotopes
TH2F *histoSpectrumO19_2D = new TH2F("O19_RT-Energy","Risetime vs. Energy (O19)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax); 
TH2F *histoSpectrumF20_2D = new TH2F("F20_RT-Energy","Risetime vs. Energy (F20)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
TH2F *histoSpectrumF21_2D = new TH2F("F21_RT-Energy","Risetime vs. Energy (F21)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
TH2F *histoSpectrumF22_2D = new TH2F("F22_RT-Energy","Risetime vs. Energy (F22)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
TH2F *histoSpectrumNe20_2D = new TH2F("Ne20_RT-Energy","Risetime vs. Energy (Ne20)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
// Other

TH2F *histoSpectrumNeutronElastic_2D = new TH2F("Neutron_Elastic_Spectrum_(2D)","Risetime vs. Amplitude (NE only)", 180,0,600000,150,0,400);  
TH2F *histoSpectrumNeutronInelastic_2D = new TH2F("Neutron_Inelastic_Spectrum_(2D)","Neutron Inelastic Spectrum (2D)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
TH2F *histoSpectrumOther_2D = new TH2F("Other_RT-Energy","Risetime vs. Energy (Others)", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax);  
// Neutron Elastic Plot
//TH2F *neutronElastic_2D = new TH2F("Neutron Elastic Scattering", "Neutron Elastic Scattering", int(Emax/binDivisorX), 0, Emax, int (RTmax/binDivisorY), 0., RTmax); 

//string TimePart = "All" ;
//string outRootName = "plots.root";

TH2F * radiushistogram = new TH2F("Radius-RiseTime_(All Events)","Radius vs. RiseTime (All Events)",675,0,675,500,0,500);
TH2F * radiushistogramNE = new TH2F("Radius-RiseTime_(NE)", "Radius vs. RiseTime (NE)",675,0,675,500,0,500);
TH2F * radiushistogramNEsigma = new TH2F("Sigma(Radius)-RiseTime_(NE)", "Sigma(Radius) vs. RiseTime (NE)",675,0,675,500,0,500);

//---------------------------------------------------------------------------------//

TFile* rootfiles = new TFile(outputfile,"RECREATE");

//---------------------------------------------------------------------------------//
// Tree to dump relevant, graphable data into
TTree * tree = new TTree("Events","Events");

tree->Branch("EventNb", &eventNb, "eventNb/I"); 
tree->Branch("firstParticleName_ref", &firstParticleName_ref, "firstParticleName_ref/I");
tree->Branch("PrimaryElectrons", &m_SumPrimary, "sumPrimary/D"); 
tree->Branch("EnergyDeposit", &m_SumSecondary, "sumSecondary/D");
tree->Branch("Risetime", &risetime, "risetime/D");
tree->Branch("EventType_ref", &eventType_ref, "eventType_ref/I"); 
tree->Branch("Amplitude", &maxhisto, "amplitude/D"); 
tree->Branch("Radius_Mean", &radius_average, "radiusAverage/D");
tree->Branch("Radius_StDev", &radius_stdev, "radiusStdev/D");





ifstream data(inputfile, ios::in);

//---------------------------------------------------------------------------------//

while(!data.eof())
	{ 
	 		nline++;
            if (nline%10000 == 0) cout << "Reading of line " << nline << endl;

			data >>
			eventNb >> 
			subSubEventNb >> 
			firstParticleName >> 
			//TimeParticleName_prev >> 
			//TimeParticleRadius_prev >>
			//FirstParticleName_prev >>
			//FirstParticleInitEkin_prev >> 
			//Process_prev >> 
			//RFStep_prev >> 
			EdepEvent >>
			m_SumPrimary >> 
			m_SumSecondaryEV >>
			risetime >>
			eventType >>
            maxhisto >> 
            radius_average >>
            radius_stdev; 
			//secondaries_print;

			//cout << m_SumSecondaryEV << " is here!" <<endl;

			//TObjString * firstParticleName_list = new TObjString(firstParticleName);
		
			//if (nline==1) {particlelist->AddFirst(firstParticleName_list);}
			//else if (particlelist->FindObject(firstParticleName_list)==0) {particlelist->AddLast(firstParticleName_list);}

			m_SumSecondary = m_SumSecondaryEV/1000.; 

            if (firstParticleName == "neutron") {
                firstParticleName_ref = 1; 
            }
            else {
                firstParticleName_ref = 0; 
            }

            if (eventType == "neutronElastic") {
                eventType_ref = 1; 
            }
            else {
                eventType_ref = 0; 
            }

        if (risetime != 0) { //(m_SumPrimary <= 3)

            tree->Fill();
        
			histoSpectrum->Fill(m_SumSecondary);	
			histoSpectrum_2D->Fill(m_SumSecondary,risetime);	

            radiushistogram->Fill(radius_average,risetime);
			
            //if (strncmp(eventType_char, "neutronElastic", 14) == 0) 
			if (eventType =="neutronElastic") 
			{
				histoSpectrumNeutronElastic->Fill(m_SumSecondary);
				histoSpectrumNeutronElastic_2D->Fill(maxhisto,risetime);

                radiushistogramNE->Fill(radius_average,risetime);
                radiushistogramNEsigma->Fill(radius_stdev,risetime);
			}

            //else if (strncmp(eventType_char, ))
		    //else if (eventType_char=="neutronInelasticNeDecay")
			//{
			//	histoSpectrumNeutronInelastic->Fill(m_SumSecondary);
			//	histoSpectrumNeutronInelastic_2D->Fill(m_SumSecondary,risetime);
			//}
			/*
			if (firstParticleName=="gamma")
			{
				histoSpectrumGamma->Fill(m_SumSecondary);
				histoSpectrumGamma_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName=="e-")
			{
				histoSpectrumElec->Fill(m_SumSecondary);
				histoSpectrumElec_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName=="e+")
			{
				histoSpectrumPosi->Fill(m_SumSecondary);
				histoSpectrumPosi_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName=="alpha")
			{
				histoSpectrumAlpha->Fill(m_SumSecondary);
				histoSpectrumAlpha_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName=="neutron")
			{
				histoSpectrumNeutron->Fill(m_SumSecondary);
				histoSpectrumNeutron_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName=="O19")
			{
				histoSpectrumO19->Fill(m_SumSecondary);
				histoSpectrumO19_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName=="F20")
			{
				histoSpectrumF20->Fill(m_SumSecondary);
				histoSpectrumF20_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName=="F21")
			{
				histoSpectrumF21->Fill(m_SumSecondary);
				histoSpectrumF21_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName=="F22")
			{
				histoSpectrumF22->Fill(m_SumSecondary);
				histoSpectrumF22_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName=="Ne20")
			{
				histoSpectrumNe20->Fill(m_SumSecondary);
				histoSpectrumNe20_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName.BeginsWith("C")==true)
			{
				histoSpectrumC->Fill(m_SumSecondary);
				histoSpectrumC_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName.BeginsWith("O")==true)
			{
				histoSpectrumO->Fill(m_SumSecondary);
				histoSpectrumO_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName.BeginsWith("F")==true)
			{
				histoSpectrumF->Fill(m_SumSecondary);
				histoSpectrumF_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName.BeginsWith("Ne")==true)
			{
				histoSpectrumNe->Fill(m_SumSecondary);
				histoSpectrumNe_2D->Fill(m_SumSecondary,risetime);
			}

			if (firstParticleName.BeginsWith("Na")==true)
			{
				histoSpectrumNa->Fill(m_SumSecondary);
				histoSpectrumNa_2D->Fill(m_SumSecondary,risetime);
			}

			if ((firstParticleName == "neutron") && (Process_prev == "hadElastic"))
			{
				neutronElastic_2D->Fill(m_SumSecondary,risetime);
			}
			*/
			
			else {
				histoSpectrumOther->Fill(m_SumSecondary);
				histoSpectrumOther_2D->Fill(m_SumSecondary,risetime);
			}

	    }
}

//particlelist->Print();

//---------------------------------------------------------------------------------//



//---------------------------------------------------------------------------------//
// Label the axes! 

// 1-D
histoSpectrum->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumRT32->GetXaxis()->SetTitle("Energy Deposit (keV)"); 

histoSpectrumGamma->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumElec->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumPosi->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumAlpha->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumNeutron->GetXaxis()->SetTitle("Energy Deposit (keV)"); 

histoSpectrumO19->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumF20->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumF21->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumF22->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumNe20->GetXaxis()->SetTitle("Energy Deposit (keV)"); 

histoSpectrumC->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumO->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumF->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumNe->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumNa->GetXaxis()->SetTitle("Energy Deposit (keV)");

histoSpectrumOther->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumNeutronElastic->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumNeutronInelastic->GetXaxis()->SetTitle("Energy Deposit (keV)");

//---------------------------------------------------------------------------------//
// 2-D
histoSpectrum_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 

histoSpectrumGamma_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumElec_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumPosi_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumAlpha_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumNeutron_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 

histoSpectrumO19_2D->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumF20_2D->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumF21_2D->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumF22_2D->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumNe20_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 

histoSpectrumC_2D->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumO_2D->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumF_2D->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumNe_2D->GetXaxis()->SetTitle("Energy Deposit (keV)");
histoSpectrumNa_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 

histoSpectrumOther_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 
histoSpectrumNeutronElastic_2D->GetXaxis()->SetTitle("Amplitude"); 
histoSpectrumNeutronInelastic_2D->GetXaxis()->SetTitle("Energy Deposit (keV)"); 

//neutronElastic_2D->GetXaxis()->SetTitle("Energy Deposit (keV)");
//histoSpectrum_2DColor->GetXaxis()->SetTitle("Energy Deposit (keV)"); 

radiushistogram->GetXaxis()->SetTitle("Average Radius (cm)"); 
radiushistogramNE->GetXaxis()->SetTitle("Average Radius (cm)"); 

radiushistogramNEsigma->GetXaxis()->SetTitle("Radius StDev (cm)"); 

//---------------------------------------------------------------------------------//

histoSpectrum_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 

histoSpectrumGamma_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumElec_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumPosi_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumAlpha_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumNeutron_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 

histoSpectrumO19_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumF20_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumF21_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumF22_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumNe20_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 

histoSpectrumC_2D->GetYaxis()->SetTitle("Rise Time (#mus)");
histoSpectrumO_2D->GetYaxis()->SetTitle("Rise Time (#mus)");
histoSpectrumF_2D->GetYaxis()->SetTitle("Rise Time (#mus)");
histoSpectrumNe_2D->GetYaxis()->SetTitle("Rise Time (#mus)");
histoSpectrumNa_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 

histoSpectrumOther_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumNeutronElastic_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 
histoSpectrumNeutronInelastic_2D->GetYaxis()->SetTitle("Rise Time (#mus)"); 

//neutronElastic_2D->GetYaxis()->SetTitle("Rise Time (#mus)");
//histoSpectrum_2DColor->GetYaxis()->SetTitle("Rise Time (#mus)"); 

radiushistogram->GetYaxis()->SetTitle("Risetime (#mus)"); 
radiushistogramNE->GetYaxis()->SetTitle("Risetime (#mus)"); 

radiushistogramNEsigma->GetYaxis()->SetTitle("Risetime (#mus)"); 

//---------------------------------------------------------------------------------//
// Save the histograms to the ROOT file

histoSpectrum 			->Write();	

/*
histoSpectrumRT32 		->Write();	

histoSpectrumGamma 		->Write();	
histoSpectrumElec 		->Write();	
histoSpectrumPosi 		->Write();
histoSpectrumAlpha 		->Write();	
histoSpectrumNeutron	->Write();

histoSpectrumO19     ->Write();
histoSpectrumF20     ->Write();
histoSpectrumF21     ->Write();
histoSpectrumF22     ->Write();
histoSpectrumNe20     ->Write();

histoSpectrumC     	->Write();
histoSpectrumO     	->Write();
histoSpectrumF     	->Write();
histoSpectrumNe     ->Write();
histoSpectrumNa     ->Write();
*/
histoSpectrumOther 	->Write();
histoSpectrumNeutronElastic ->Write();
//histoSpectrumNeutronInelastic ->Write();  

//---------------------------------------------------------------------------------//
const char* option = "colz";


histoSpectrum_2D 		->SetOption(option);	

histoSpectrumGamma_2D 	->SetOption(option);	
histoSpectrumElec_2D 	->SetOption(option);	
histoSpectrumPosi_2D 	->SetOption(option);	
histoSpectrumAlpha_2D 	->SetOption(option);	
histoSpectrumNeutron_2D ->SetOption(option);

histoSpectrumO19_2D   ->SetOption(option);
histoSpectrumF20_2D   ->SetOption(option);
histoSpectrumF21_2D   ->SetOption(option);
histoSpectrumF22_2D   ->SetOption(option);
histoSpectrumNe20_2D   ->SetOption(option);

histoSpectrumC_2D   	->SetOption(option);
histoSpectrumO_2D   	->SetOption(option);
histoSpectrumF_2D   	->SetOption(option);
histoSpectrumNe_2D   	->SetOption(option);
histoSpectrumNa_2D   	->SetOption(option);

histoSpectrumOther_2D 	->SetOption(option);
histoSpectrumNeutronElastic_2D->SetOption(option);
histoSpectrumNeutronInelastic_2D->SetOption(option); 

//neutronElastic_2D		->SetOption(option); 

//---------------------------------------------------------------------------------//
// last-minute tuning

histoSpectrum_2D->SetMaximum(50); 
histoSpectrumOther_2D->SetMaximum(50); 
histoSpectrumNeutronElastic_2D->SetMaximum(50); 

//---------------------------------------------------------------------------------//

histoSpectrum_2D 		->Write();	
/*
histoSpectrumGamma_2D 	->Write();	
histoSpectrumElec_2D 	->Write();	
histoSpectrumPosi_2D 	->Write();	
histoSpectrumAlpha_2D 	->Write();	
histoSpectrumNeutron_2D ->Write();

histoSpectrumO19_2D   ->Write();
histoSpectrumF20_2D   ->Write();
histoSpectrumF21_2D   ->Write();
histoSpectrumF22_2D   ->Write();
histoSpectrumNe20_2D  ->Write();

histoSpectrumC_2D   	->Write();
histoSpectrumO_2D   	->Write();
histoSpectrumF_2D   	->Write();
histoSpectrumNe_2D   	->Write();
histoSpectrumNa_2D   	->Write();
*/
histoSpectrumOther_2D 	->Write();

histoSpectrumNeutronElastic_2D->Write();
//histoSpectrumNeutronInelastic_2D->Write();


radiushistogram->Write(); 
radiushistogramNE->Write(); 

radiushistogramNEsigma->Write();

tree->Write(); 

//neutronElastic_2D		->Write();

//---------------------------------------------------------------------------------//

histoSpectrumGamma_2D ->SetMarkerColor(1);	
histoSpectrumElec_2D 	->SetMarkerColor(2);	
histoSpectrumPosi_2D 	->SetMarkerColor(3);		
histoSpectrumAlpha_2D ->SetMarkerColor(4);		
histoSpectrumNeutron_2D ->SetMarkerColor(5);	
histoSpectrumO_2D   ->SetMarkerColor(6);	
histoSpectrumF_2D   ->SetMarkerColor(7);	
histoSpectrumNe_2D  ->SetMarkerColor(8);	
histoSpectrumNa_2D  ->SetMarkerColor(9);	

//---------------------------------------------------------------------------------//

 
 

// Print out some graphs to look at (the most important ones have already been saved)

//TCanvas *canvas = new TCanvas("RT-Energy","Risetime vs. Energy",0,0,1500,1000);
//canvas->Divide(2,1);

//canvas->cd();


/*
histoSpectrumGamma_2D ->SetOption("colz");	
//histoSpectrumGamma_2D ->GetXaxis()->SetRangeUser(0,500000);
//histoSpectrumGamma_2D
histoSpectrumGamma_2D 	->Draw();	
histoSpectrumElec_2D 	->Draw("same");			
histoSpectrumPosi_2D 	->Draw("same");				
histoSpectrumAlpha_2D	->Draw("same");		
histoSpectrumNeutron_2D ->Draw("same");		
histoSpectrumO_2D   	->Draw("same");	
histoSpectrumF_2D 		->Draw("same");		
histoSpectrumNe_2D  	->Draw("same");		
histoSpectrumNa_2D  	->Draw("same");		


// Build the legend to display
TLegend * legend = new TLegend(0.94,0.6,0.85,0.25); 
legend->SetHeader("Legend","C"); 

TLegendEntry *entry1 = legend->AddEntry(histoSpectrumGamma_2D, "#gamma", "");
entry1->SetTextColor(1); entry1->SetMarkerSize(5); 
TLegendEntry *entry2 = legend->AddEntry(histoSpectrumElec_2D, "e^{-}", "");
entry2->SetTextColor(2); entry2->SetMarkerSize(5); 
TLegendEntry *entry3 = legend->AddEntry(histoSpectrumPosi_2D, "e^{+}", "");
entry3->SetTextColor(3); entry3->SetMarkerSize(5); 
TLegendEntry *entry4 = legend->AddEntry(histoSpectrumAlpha_2D, "#alpha", "");
entry4->SetTextColor(4); entry4->SetMarkerSize(5); 
TLegendEntry *entry5 = legend->AddEntry(histoSpectrumNeutron_2D, "n", "");
entry5->SetTextColor(5); entry5->SetMarkerSize(5); 
TLegendEntry *entry6 = legend->AddEntry(histoSpectrumO_2D, "O", "");
entry6->SetTextColor(6); entry6->SetMarkerSize(5); 
TLegendEntry *entry7 = legend->AddEntry(histoSpectrumF_2D, "F", "");
entry7->SetTextColor(7); entry7->SetMarkerSize(5); 
TLegendEntry *entry8 = legend->AddEntry(histoSpectrumNe_2D, "Ne", "");
entry8->SetTextColor(8); entry8->SetMarkerSize(5); 
TLegendEntry *entry9 = legend->AddEntry(histoSpectrumNa_2D, "Na", "");
entry9->SetTextColor(9); entry9->SetMarkerSize(5); 
*/

//histoSpectrum_2DColor->Draw();
//legend->Draw(); 


//canvas->cd(2);

// Save!!!
rootfiles->Save();

return 0;
}
