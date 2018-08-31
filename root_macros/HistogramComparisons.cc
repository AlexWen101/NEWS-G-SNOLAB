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

#define nbfiles 7

using namespace std; 

int xMax(200000);
int yMax(1000);

int xBins(200);
int yBins(150); 

double zMax(0.0035); // this is normalized 

double radius_threshold(650); 

// This macro consolidates all the data into one root file called data_summary.root and draws some histograms of interst. 

int eventNb; 
int firstParticleName_ref; 
double m_SumPrimary;
double m_SumSecondary; 
double risetime; 
int eventType_ref; 
double maxhisto; 
double radius_average; 
double radius_stdev; 


TH2F * histopointers[nbfiles];

TH2F * histopointers_neutronelastic[nbfiles];
TH2F * histopointers_neutroninelastic[nbfiles];

TH2F * histopointers_surface[nbfiles];
TH2F * histopointers_volume[nbfiles];

TString histonames[nbfiles];

TTree * treepointers[nbfiles]; 
TString treenames[nbfiles]; 

TFile * filepointers[nbfiles];
TString filenames[nbfiles] = {
    "AmBe_L1.root",
    "AmBe_L2.root", 
    "AmBe_L6.root",
    "DD_L1.root",
    "DD_L2.root",
    "DD_L6.root",
    "AmBe_L5.root"
};


int HistogramComparisons() {


TFile * datafile = new TFile("data_summary.root","RECREATE");

TH2F * sample_plot = new TH2F("sample","Simple",200,0,500000,150,0,800); 

// copy over the trees and create necessary pointers to everything
for (int i = 0; i < nbfiles; i++) {
    
    // make histogram names

    histonames[i] = filenames[i]; 
    histonames[i].Remove(histonames[i].Length()-5,5);
    
    // make histograms

    histopointers[i] = new TH2F(histonames[i]+"_normalized_colz",histonames[i],xBins,0,xMax,yBins,0,yMax); 

    histopointers_neutronelastic[i] = new TH2F(histonames[i]+"_neutronelastic",histonames[i],170,0,50000,150,0,300);
    histopointers_neutroninelastic[i] = new TH2F(histonames[i]+"_neutroninelastic",histonames[i],xBins,0,xMax,yBins,0,yMax); 

    histopointers_surface[i] = new TH2F(histonames[i]+"_surface",histonames[i],xBins,0,xMax,yBins,0,yMax);
    histopointers_volume[i] = new TH2F(histonames[i]+"_volume",histonames[i],xBins,0,xMax,yBins,0,yMax); 

    

    // make the tree names

    treenames[i] = filenames[i]; 
    treenames[i].Replace(treenames[i].Length()-5,5,"_tree");

    // make the file pointers

    filepointers[i] = new TFile(filenames[i],"READ"); 

    //filepointers[i]->ls(); 
    treepointers[i] = (TTree*)filepointers[i]->Get("Events"); 
    
    datafile->cd();
    treepointers[i]->SetName(treenames[i]); 
    treepointers[i]->CloneTree()->Write();
    
}



// make the histograms
for (int i = 0; i < nbfiles; i++) {

    treepointers[i]->SetBranchAddress("EventNb",&eventNb); 
    treepointers[i]->SetBranchAddress("firstParticleName_ref",&firstParticleName_ref);
    treepointers[i]->SetBranchAddress("PrimaryElectrons",&m_SumPrimary);
    treepointers[i]->SetBranchAddress("EnergyDeposit",&m_SumSecondary);
    treepointers[i]->SetBranchAddress("Risetime",&risetime);
    treepointers[i]->SetBranchAddress("EventType_ref",&eventType_ref);
    treepointers[i]->SetBranchAddress("Amplitude",&maxhisto);
    treepointers[i]->SetBranchAddress("Radius_Mean",&radius_average);
    treepointers[i]->SetBranchAddress("Radius_StDev",&radius_stdev);

    if (i==6) {
        for (int j = 0; j < treepointers[i]->GetEntries(); j++) {
            treepointers[i]->GetEntry(j); 
            sample_plot->Fill(maxhisto,risetime);
        }
        sample_plot->SetStats(kFALSE);
        sample_plot->SetOption("colz");
        sample_plot->GetXaxis()->SetTitle("Amplitude (ADU)"); 
        sample_plot->GetYaxis()->SetTitle("Rise Time (#mus)");
        sample_plot->Write(); 
    }

    for (int j = 0; j < treepointers[i]->GetEntries(); j++) {


        treepointers[i]->GetEntry(j); 

        histopointers[i]->Fill(maxhisto,risetime);   

        if (radius_average > radius_threshold) {
            histopointers_surface[i]->Fill(maxhisto,risetime);
        }
        else {
            histopointers_volume[i]->Fill(maxhisto,risetime); 
        }

        if (eventType_ref == 1) {
            histopointers_neutronelastic[i]->Fill(maxhisto,risetime); 
        }
        else {
            histopointers_neutroninelastic[i]->Fill(maxhisto,risetime); 
        }

    }

    histopointers[i]->Scale(1/histopointers[i]->GetEntries()); 
    histopointers[i]->SetTitle(histonames[i]+"_normalized_colz"); 
    histopointers[i]->SetMaximum(zMax); 

    histopointers[i]->GetXaxis()->SetTitle("Amplitude (eV)");
    histopointers[i]->GetYaxis()->SetTitle("Rise Time (#mus)");

    histopointers[i]->SetOption("colz"); 
     histopointers_neutronelastic[i]->SetOption("col"); 

    histopointers[i]->Write(); 


    histopointers_surface[i]->SetTitle(histonames[i]+"_surface");
    histopointers_volume[i]->SetTitle(histonames[i]+"_volume"); 

    histopointers_neutronelastic[i]->SetTitle(histonames[i]+"_elastic");
    histopointers_neutroninelastic[i]->SetTitle(histonames[i]+"_inelastic");

    histopointers_surface[i]->GetXaxis()->SetTitle("Amplitude (eV)");
    histopointers_surface[i]->GetYaxis()->SetTitle("Rise Time (#mus)");

    histopointers_volume[i]->GetXaxis()->SetTitle("Amplitude (eV)");
    histopointers_volume[i]->GetYaxis()->SetTitle("Rise Time (#mus)");


    histopointers_surface[i]->Write(); 
    histopointers_volume[i]->Write();
    
    histopointers_neutronelastic[i]->Write(); 
    histopointers_neutroninelastic[i]->Write(); 

}

gStyle->SetOptStat("e"); 


/*
//----------------------------------------------------------------------------------//
// create canvases and new file
TCanvas * mainCanvas = new TCanvas("mainCanvas","main",1400,1000);
TCanvas * L5Canvas = new TCanvas("L5Canvas","L5",700,500); 

//TFile * newfile = new TFile("NeutronElasticComparisons.root","RECREATE")

//----------------------------------------------------------------------------------//
// Get data files
TFile * AmBeL1 = new TFile("AmBe_L1.root","READ");
TFile * AmBeL2 = new TFile("AmBe_L2.root","READ");
TFile * AmBeL6 = new TFile("AmBe_L6.root","READ");
TFile * DDL1 = new TFile("DD_L1.root","READ");
TFile * DDL2 = new TFile("DD_L2.root","READ");
TFile * DDL6 = new TFile("DD_L6.root","READ");

TFile * AmBeL5 = new TFile("AmBeNeutron_L5.root","READ"); 

//----------------------------------------------------------------------------------//
// Get trees from the files
TTree * AmBeL1_tree = (TTree*)AmBeL1->Get("Events");
TTree * AmBeL2_tree = (TTree*)AmBeL2->Get("Events");
TTree * AmBeL6_tree = (TTree*)AmBeL6->Get("Events");
TTree * DDL1_tree = (TTree*)DDL1->Get("Events");
TTree * DDL2_tree = (TTree*)DDL2->Get("Events");
TTree * DDL6_tree = (TTree*)DDL6->Get("Events");

TTree * AmBeL5_h = (TTree*)AmBeL5->Get("Events");

//----------------------------------------------------------------------------------//
// Set graph titles
AmBeL1_h->SetTitle("AmBe, L1, Elastic Neutrons, Normalized");
AmBeL2_h->SetTitle("AmBe, L2, Elastic Neutrons, Normalized");
DDL1_h->SetTitle("DD, L1, Elastic Neutrons, Normalized"); 
DDL2_h->SetTitle("DD, L2, Elastic Neutrons, Normalized");

AmBeL5_h->SetTitle("AmBe, L5 (calibration), Elastic Neutrons, Normalized");

//----------------------------------------------------------------------------------//
// set range on x axis
AmBeL1_h->GetXaxis()->SetRangeUser(0,xMax);
AmBeL2_h->GetXaxis()->SetRangeUser(0,xMax); 
DDL1_h->GetXaxis()->SetRangeUser(0,xMax);
DDL2_h->GetXaxis()->SetRangeUser(0,xMax);

AmBeL5_h->GetXaxis()->SetRangeUser(0,xMax);

//----------------------------------------------------------------------------------//
// set range on y axis
AmBeL1_h->GetYaxis()->SetRangeUser(0,yMax);
AmBeL2_h->GetYaxis()->SetRangeUser(0,yMax); 
DDL1_h->GetYaxis()->SetRangeUser(0,yMax);
DDL2_h->GetYaxis()->SetRangeUser(0,yMax);

AmBeL5_h->GetYaxis()->SetRangeUser(0,yMax);

//----------------------------------------------------------------------------------//
// normalize histograms
AmBeL1_h->Scale(1/(AmBeL1_h->GetEntries()));
AmBeL2_h->Scale(1/(AmBeL2_h->GetEntries()));
DDL1_h->Scale(1/(DDL1_h->GetEntries()));
DDL2_h->Scale(1/(DDL2_h->GetEntries()));

AmBeL5_h->Scale(1/(AmBeL5_h->GetEntries()));

//----------------------------------------------------------------------------------//
// set max number of entries per bin
AmBeL1_h->SetMaximum(zMax); 
AmBeL2_h->SetMaximum(zMax);  
DDL1_h->SetMaximum(zMax);
DDL2_h->SetMaximum(zMax);

AmBeL5_h->SetMaximum(zMax);

//----------------------------------------------------------------------------------//
// set stat box style
gStyle->SetOptStat("e"); 

//----------------------------------------------------------------------------------//


mainCanvas->Divide(2,2);

L5Canvas->cd(); 
AmBeL5_h->Draw(); 
 
mainCanvas->cd(1); 
AmBeL1_h->Draw(); 
mainCanvas->cd(2);
AmBeL2_h->Draw();
mainCanvas->cd(3);
DDL1_h->Draw();
mainCanvas->cd(4);
DDL2_h->Draw(); 
*/
return 0; 
}
