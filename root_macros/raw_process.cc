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

// this file is designed to take in the raw .txt file and sort all the steps in each event in order, plus add some stuff that 
// quadis needs as input. The output of this macro is fed directly into quadis. 

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
    double globalTime, globalTime_relative; 
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


TString timeParticleFirst("firstEvent"); 
double positionTimeXfirst(0);
double positionTimeYfirst(0);
double positionTimeZfirst(0);

double poststepx(0), poststepy(0), poststepz(0); 
double timeparticlex(0), timeparticley(0), timeparticlez(0); 
double energydifference(0); 
double globaltime(0); 

double globaltime_relative_max(0); 


//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//
// Primary()
//---------------------------//---------------------------//---------------------------//---------------------------//---------------------------//

int Primary(string inputfile, string outputfile) {

    // Inputfile: user inputted name as the sold argument of the function
    ifstream infile(inputfile,std::ifstream::in);
    // Outputfile: automatically named (the same name that Primary Processing reads)
    ofstream outfile(outputfile,std::ifstream::trunc);

    outfile <<
    "	Event_Id	" <<
    "   Pulse_Time	" << 
    "	Particle_Name	" << 
    "	Particle_InitiEK	"<<
    "	ProcessCreator	"<<
    "	Particle_ID	" <<
    "	Mother_Name	" <<
    "	Mother_ID	" <<
    "	Pre_step_point_x	" <<
	"	Pre_step_point_y	" <<
	"	Pre_step_point_z	" <<
    "	Post_step_point_x	" <<
    "	Post_step_point_y	" <<
    "	Post_step_point_z	" <<
    "	Energy_dep_step_Io " 	<<
    "	Energy_part	" <<
    "	Kin_Energy_part	" <<
    "	Process	" <<
    "	Time_Particle	"<<
    "	TimeParticle_first_step_x	"<<
    "	TimeParticle_first_step_y	"<<
    "	TimeParticle_first_step_z	"<<
    "   SubEvent_Id   "
    << endl; 


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


        // unit conversion for relevant quantities. Everything should be in mm, keV, and micro s
        data_current.stepEnergyDifference = data_current.stepEnergyDifference*1000; 
        data_current.globalTime = data_current.globalTime * 1000000; 
        data_current.globalTime_relative = data_current.globalTime; 

        if (data_current.positionPostX > poststepx) poststepx = data_current.positionPostX; 
        if (data_current.positionPostY > poststepy) poststepy = data_current.positionPostY;
        if (data_current.positionPostZ > poststepz) poststepz = data_current.positionPostZ;

        if (data_current.positionTimeX > timeparticlex) timeparticlex = data_current.positionTimeX;
        if (data_current.positionTimeY > timeparticley) timeparticley = data_current.positionTimeY;
        if (data_current.positionTimeZ > timeparticlez) timeparticlez = data_current.positionTimeZ;

        if (data_current.stepEnergyDifference > energydifference) energydifference = data_current.stepEnergyDifference;
        if (data_current.globalTime > globaltime) globaltime = data_current.globalTime;

        //cout << data_current.positionPost[2] << " at " << nline << endl; 
        

        if (eventNb_pre == data_current.eventNb) {
            /*
            timeParticleFirst = data_current.timeParticle;
            positionTimeXfirst = data_current.positionTimeX;
            positionTimeYfirst =  data_current.positionTimeY;
            positionTimeZfirst = data_current.positionTimeZ;
            */
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
                    if ((data_vector[k].globalTime - data_vector[k-1].globalTime) > 1000)  {

                        subSubEventNb_int++;

                        timeParticleFirst = data_vector[k].particleName; 
                        
                        positionTimeXfirst = data_vector[k].positionPreX;
                        positionTimeYfirst = data_vector[k].positionPreY;
                        positionTimeZfirst = data_vector[k].positionPreZ;

                        for (int m = k; m < size; m++) {
                            data_vector[m].globalTime_relative = data_vector[m].globalTime - data_vector[k].globalTime; 

                            if (data_vector[m].globalTime_relative > globaltime_relative_max) globaltime_relative_max = data_vector[m].globalTime_relative;
                        } 


                    }
                }



                outfile <<" "<<
                data_vector[k].eventNb <<" "<<
                data_vector[k].globalTime_relative <<" "<<
                data_vector[k].particleName <<" "<<
                //data_vector[k].particleType <<" "<< 
                data_vector[k].initialEkin <<" "<<
                data_vector[k].processCreator <<" "<<
                data_vector[k].particleNb <<" "<<
                data_vector[k].motherName <<" "<<
                data_vector[k].motherNb <<" "<<
                data_vector[k].positionPreX <<" "<<
                data_vector[k].positionPreY <<" "<<
                data_vector[k].positionPreZ <<" "<<
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
                timeParticleFirst <<" "<<
                positionTimeXfirst <<" "<<
                positionTimeYfirst <<" "<<
                positionTimeZfirst <<" "<<
                //data_vector[k].subEventNb <<" "<<
                subSubEventNb_int << endl; 
            
            }

            data_vector.clear();

            data_vector.push_back(data_current); 

            timeParticleFirst = data_current.particleName; 
            positionTimeXfirst = data_current.positionPreX; 
            positionTimeYfirst = data_current.positionPreY; 
            positionTimeZfirst = data_current.positionPreZ; 

        }

        eventNb_pre = data_current.eventNb;

        globalTime_pre = data_current.globalTime; 

        
        


    }

cout << "You just processed the total events." << endl; 
cout << "Max post step coordinates: " << poststepx << ", " << poststepy << ", " << poststepz << endl; 
cout << "Max timepartcle coordinates: " << timeparticlex << ", " << timeparticley << ", " << timeparticlez << endl; 
cout << "Max energy dep: " << energydifference <<endl; 
cout << "Max global time " << globaltime_relative_max << endl;  

return 0;

}