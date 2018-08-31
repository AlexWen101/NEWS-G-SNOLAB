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

#include <stdio.h>
#include <stdlib.h>

#include "Riostream.h"
#include "TString.h"
#include "TSystem.h"

#include "TMath.h"
#include <stdbool.h>
#include <locale.h>

// Draws the energy spectrum of the AmBe source neutrons from data in the "Data" folder of the g4 simulation
// (you need to copy the am-be_neutron_spectrum.txt file over from that folder.)

double energy; 
double freq; 

int ambe_spectrum() {

ifstream infile("am-be_neutron_spectrum.txt",std::ifstream::in);

TH1D * histo = new TH1D("Energy","",105,0,11.5); 
TCanvas * canvas = new TCanvas("canvas","canvas",1000,700); 

while(!infile.eof()) {

    infile >> 
    energy >> 
    freq; 

    histo->Fill(energy,freq); 

}

canvas->cd(); 

gStyle->SetOptStat(""); 

histo->GetYaxis()->SetTitle("Relative Counts");
histo->GetXaxis()->SetTitle("Neutron Energy (MeV)");  

histo->Draw("hist C");

return 0;
}