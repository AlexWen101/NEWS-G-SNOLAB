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
#include <TGraph.h>
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
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "TMath.h"
#include <stdbool.h>
#include <locale.h>

// this macro plots some data for the percentage of gammas reacting in the sensitive volume 
// it investigates different thicknesses of lead and how gammas from the AmBe source will
// be shielded, and also plots the effect on neutron counts. 
// all the values contained in these plots are taken from the data found on Sphere: 
//      /home/awen/SSData/data_gamma


int gamma_graphs() {

    //-------------------// //-------------------// //-------------------// //-------------------// //-------------------// //-------------------// 

    Int_t n = 9; 

    Double_t pb_thickness[9] = {10,20,30,40,50,70,90,110,130};
    Double_t percent_events[9] = {22.364,12.010,7.633,3.531,2.625,1.051,0.437,0.216,0.075}; 

    TGraph * percent_vs_thickness = new TGraph(n, pb_thickness, percent_events); 
    TCanvas * c1 = new TCanvas("c1","c1",800,600);

    percent_vs_thickness->GetYaxis()->SetTitle("Gamma Events/Total Gammas Simulated (%)");
    percent_vs_thickness->GetXaxis()->SetTitle("Pb Shield Thickness (mm)");
    percent_vs_thickness->SetTitle("Percentage of Gammas Creating Events by Pb Thickness");

    percent_vs_thickness->SetLineColor(4);

    c1->cd(); 

    percent_vs_thickness->Draw("AC*"); 

    //-------------------// //-------------------// //-------------------// //-------------------// //-------------------// //-------------------// 

    const Int_t nx = 6; 

    const char * labels[nx] = {"Location 5",  "30 mm Pb", "50 mm Pb", "Location 6", "Location 1", "Location 2"};

    Double_t percent_events_n[nx] = {5.81,6.05,5.50,3.29,2.07,2.09};

    TH1D * percent_vs_location = new TH1D("histo","histo",nx,0,nx); 
    TCanvas * c2 = new TCanvas("c2","c2",1000,750); 

    for (int i = 0; i < nx; i++) {
        percent_vs_location->SetBinContent(i+1,percent_events_n[i]);
        percent_vs_location->GetXaxis()->SetBinLabel(i+1,labels[i]); 
    }

    c2->cd(); 

    gStyle->SetOptStat(""); 

    percent_vs_location->SetTitle("Percentage of Neutrons Creating Events by Location"); 
    percent_vs_location->GetXaxis()->SetTitle("Location/Pb Shield Thickness (ascending order of distance to sphere)"); 
    percent_vs_location->GetYaxis()->SetTitle("Neutron Events/Total Neutrons Simulated (%)"); 


    percent_vs_location->SetBarWidth(0.5);
    percent_vs_location->SetBarOffset(0.25); 
    percent_vs_location->SetFillColor(4); 
    percent_vs_location->Draw("b"); 

    

    return 0; 
}