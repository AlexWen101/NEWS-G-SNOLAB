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

#define nbfiles 6

#define nbcuts 4

using namespace std; 

int xMax(200);
int yMax(400);
int xMaxADU(500000); 

int xBins(200);
int yBins(150); 

/* 

This macro takes in a bunch of ROOT data files generated with QUADIS root2DD2vector and consolidates them into one, and 
also fills a bunch of histograms and puts everything into trees. 

This is really horribly written and all over the place. Documentation is bad too. If you need to change something from here, I suggest figuring out the 
pointer to the histogram you're interested in, and seeing where that pointer is called so you know all the lines of code that correspond to one histogram/canvas/object. 

The stuff is all over the place, so trying to organize it line by line is horribly difficult...

*/




// this is an array of pointers that draw default histograms, plain and simple, for comparison. 
TH2F * histopointers[nbfiles];

// these are useless. 
TH2F * histopointers_neutronelastic[nbfiles];
TH2F * histopointers_neutroninelastic[nbfiles];

// these are also useless. 
TH2F * histopointers_surface[nbfiles];
TH2F * histopointers_volume[nbfiles];

// an array of TStrings so that histogram titles can be set quickly based on their file names. 
TString histonames[nbfiles];

// for copying over all the trees from the separate data files. 
TTree * treepointers[nbfiles]; 
TString treenames[nbfiles]; 

// for keeping track of all the files to be read in. 
TFile * filepointers[nbfiles];
TString filenames[nbfiles] = {
    "sh13s000-G4Sim-AmBe-L1_DD2_q00.root",
    //"sh13s000-G4Sim-AmBe-L2_DD2_q00.root", 
    "sh09s000-G4Sim-AmBe-L5_DD2_q00.root",
    "sh15s000-G4Sim-AmBe-L6_DD2_q00.root",
    "sh15s000-G4Sim-DD-L1_DD2_q00.root",
    //"sh16s000-G4Sim-DD-L2_DD2_q00.root",
    "sh10s000-G4Sim-DD-L5_DD2_q00.root",
    "sh17s000-G4Sim-DD-L6_DD2_q00.root"
};

// these are 1D projections of histopointers[] histograms on the horizontal (ampl) 
// and vertical (rt) axis. 
TH1D * proj_ampl_ambe;
TH1D * proj_rt_ambe; 
TH1D * proj_ampl_dd;
TH1D * proj_rt_dd; 

// also projections like above, but for the graph with a lot of horizontal slices 
TH1D * proj_ampl_ambe_a[5];
TH1D * proj_ampl_dd_a[5];  
TH2F * ampl_ambe_clones[5];
TH2F * ampl_dd_clones[5]; 

// histograms for displaying the signal, and breaking down the signal by process (eIonization, hadElastic, etc)
// and then printing out their horizontal projections. 
TH2D * histopointers_cuts[nbcuts]; 
TH1D * histopointers_cuts_proj[nbcuts]; 

//shitload of declarations (nothing for now needs all these different variables, but i guess you never know ¯\_(ツ)_/¯)

   vector<double> * TimePartId;
   vector<double> * ProcessCreatorId;
   vector<double> * MainSteps;
   vector<double> * TimeRho;
   vector<double> * TimeZ;

   vector<double> * PartNameId;
   vector<double> * ProcessId;
   vector<double> * Energy;
   vector<double> * Rho;
   vector<double> * Z;

   vector<double> * NElec;
   vector<double> * NElecReached;
   vector<double> * NElecSecondary;
   vector<double> * StartTime;
   vector<double> * EndTime;

   vector<double> * NoiseTrigger;
   vector<double> * PulseTrigger;
   vector<double> * NoiseEntry;
   vector<double> * Channel;
   vector<double> * DD_Baseline;

   vector<double> * DD_Baseline_RMS;
   vector<double> * DD_RawPulse0;
   vector<double> * DD_RawPulseN;
   vector<double> * DD_Smooth5Baseline;
   vector<double> * DD_Smooth5Baseline_RMS;

   vector<double> * DD_Smooth11Baseline;
   vector<double> * DD_Smooth11Baseline_RMS;
   vector<double> * DD_RawAmpl;
   vector<double> * DD_RawMaxTime;
   vector<double> * DD_RawMaxSample;

   vector<double> * DD_RawRise10pct;
   vector<double> * DD_RawRise90pct;
   vector<double> * DD_RawRise;
   vector<double> * DD_RawRise50pct;
   vector<double> * DD_RawFall50pct;

   vector<double> * DD_RawWidth;
   vector<double> * DD_StartTime;
   vector<double> * DD_StopTime;
   vector<double> * DD_ThresholdStop;
   vector<double> * DD_RealStartTime;

   vector<double> * DD_RealStopTime;
   vector<double> * DD_BaselineStart;
   vector<double> * DD_BaselineEnd;
   vector<double> * DD_Ampl;
   vector<double> * DD_AmplADU;

   vector<double> * DD_Rise;
   vector<double> * DD_Rise10pct;
   vector<double> * DD_Rise25pct;
   vector<double> * DD_Rise75pct;
   vector<double> * DD_Rise90pct;


int data_consolidator() {

TFile * datafile = new TFile("consolidated_data.root","RECREATE");

// copy over the trees and create necessary pointers to everything

TString name("AmBe L5 ");
histopointers_cuts[0] = new TH2D(name+"combined",name+"Combined",xBins,0,xMax,yBins,0,yMax);
histopointers_cuts[1] = new TH2D(name+"not hadElastic",name+"not hadElastic",xBins,0,xMax,yBins,0,yMax);
histopointers_cuts[2] = new TH2D(name+"eIoni",name+"eIoni",xBins,0,xMax,yBins,0,yMax);
histopointers_cuts[3] = new TH2D(name+"neutronInelastic",name+"neutronInelastic",xBins,0,xMax,yBins,0,yMax);

TH2F * sample_plot = new TH2F("sample","QUADIS",xBins,0,xMaxADU,yBins,0,yMax);

for (int i = 0; i < nbfiles; i++) {
    
    // make histogram names

    histonames[i] = filenames[i]; 
    histonames[i].Remove(histonames[i].Length()-13,13);
    
    // make histograms

    histopointers[i] = new TH2F(histonames[i],histonames[i],xBins,0,xMax,yBins,0,yMax); 
    

    

   // histopointers_neutronelastic[i] = new TH2F(histonames[i]+"_neutronelastic",histonames[i],170,0,50000,150,0,300); 

    // make the tree names

    treenames[i] = filenames[i]; 
    treenames[i].Replace(treenames[i].Length()-13,13,"_tree");

    // make the file pointers

    filepointers[i] = new TFile(filenames[i],"READ"); 

    //filepointers[i]->ls(); 
    treepointers[i] = (TTree*)filepointers[i]->Get("T2"); 
    
    datafile->cd();
    treepointers[i]->SetName(treenames[i]); 
    treepointers[i]->CloneTree()->Write();
    
}





TCanvas * gen_canvas = new TCanvas("gen_canvas","gen_canvas",1800,800);
gen_canvas->Divide(3,2); 

TCanvas * gen_canvas_cuts = new TCanvas("gen_canvas_cuts","gen_canvas_cuts",1500,1600);
gen_canvas_cuts->Divide(2,4); 

TCanvas * source_comparison_canvas = new TCanvas("det_canvas","det_canvas",1500,1200);
source_comparison_canvas->Divide(2,2); 

TCanvas * ampl_comparison_canvas =  new TCanvas("ampl_comparison","ampl_comparison",1500,1200); 
ampl_comparison_canvas->Divide(3,5); 
/*
double width_x = 0.3; 
int Nx = 3; 
double width_y = 0.18; 
int Ny = 5; 
double margin_x = (1-(Nx*width_x))/2; 
double dw_x = (width_x*0.1)/4;
double margin_y = (1-(Ny*width_y))/2;
double dw_y = (width_y*0.1)/4; 

TPad * padpointers[15]; 
TString padnames[15] = {"p1","p2","p3","p4","p5","p6","p7","p8","p9","p10","p11","p12","p13","p14","p15"};

    for (int t = 0; t < 3; t++) {
        padpointers[t] = new TPad(padnames[t],padnames[t], margin_x+t*(width_x+dw_x), margin_y)
    }
*/

// make the histograms
for (int i = 0; i < nbfiles; i++) {

    treepointers[i]->SetBranchAddress("TimePartId",&TimePartId); 
    treepointers[i]->SetBranchAddress("ProcessCreatorId",&ProcessCreatorId);
    treepointers[i]->SetBranchAddress("MainSteps",&MainSteps);
    treepointers[i]->SetBranchAddress("TimeRho",&TimeRho);
    treepointers[i]->SetBranchAddress("TimeZ",&TimeZ);

    treepointers[i]->SetBranchAddress("PartNameId",&PartNameId);
    treepointers[i]->SetBranchAddress("ProcessId",&ProcessId);
    treepointers[i]->SetBranchAddress("Energy",&Energy);
    treepointers[i]->SetBranchAddress("Rho",&Rho);
    treepointers[i]->SetBranchAddress("Z",&Z);

    treepointers[i]->SetBranchAddress("NElec",&NElec);
    treepointers[i]->SetBranchAddress("NElecReached",&NElecReached);
    treepointers[i]->SetBranchAddress("NElecSecondary",&NElecSecondary);
    treepointers[i]->SetBranchAddress("StartTime",&StartTime);
    treepointers[i]->SetBranchAddress("EndTime",&EndTime);

    treepointers[i]->SetBranchAddress("NoiseTrigger",&NoiseTrigger);
    treepointers[i]->SetBranchAddress("PulseTrigger",&PulseTrigger);
    treepointers[i]->SetBranchAddress("NoiseEntry",&NoiseEntry);
    treepointers[i]->SetBranchAddress("Channel",&Channel);
    treepointers[i]->SetBranchAddress("DD_Baseline",&DD_Baseline);

    treepointers[i]->SetBranchAddress("DD_Baseline_RMS",&DD_Baseline_RMS);
    treepointers[i]->SetBranchAddress("DD_RawPulse0",&DD_RawPulse0);
    treepointers[i]->SetBranchAddress("DD_RawPulseN",&DD_RawPulseN);
    treepointers[i]->SetBranchAddress("DD_Smooth5Baseline",&DD_Smooth5Baseline);
    treepointers[i]->SetBranchAddress("DD_Smooth5Baseline_RMS",&DD_Smooth5Baseline_RMS);

    treepointers[i]->SetBranchAddress("DD_Smooth11Baseline",&DD_Smooth11Baseline);
    treepointers[i]->SetBranchAddress("DD_Smooth11Baseline_RMS",&DD_Smooth11Baseline_RMS);
    treepointers[i]->SetBranchAddress("DD_RawAmpl",&DD_RawAmpl);
    treepointers[i]->SetBranchAddress("DD_RawMaxTime",&DD_RawMaxTime);
    treepointers[i]->SetBranchAddress("DD_RawMaxSample",&DD_RawMaxSample);

    treepointers[i]->SetBranchAddress("DD_RawRise10pct",&DD_RawRise10pct);
    treepointers[i]->SetBranchAddress("DD_RawRise90pct",&DD_RawRise90pct);
    treepointers[i]->SetBranchAddress("DD_RawRise",&DD_RawRise);
    treepointers[i]->SetBranchAddress("DD_RawRise50pct",&DD_RawRise50pct);
    treepointers[i]->SetBranchAddress("DD_RawFall50pct",&DD_RawFall50pct);

    treepointers[i]->SetBranchAddress("DD_RawWidth",&DD_RawWidth);
    treepointers[i]->SetBranchAddress("DD_StartTime",&DD_StartTime);
    treepointers[i]->SetBranchAddress("DD_StopTime",&DD_StopTime);
    treepointers[i]->SetBranchAddress("DD_ThresholdStop",&DD_ThresholdStop);
    treepointers[i]->SetBranchAddress("DD_RealStartTime",&DD_RealStartTime);

    treepointers[i]->SetBranchAddress("DD_RealStopTime",&DD_RealStopTime);
    treepointers[i]->SetBranchAddress("DD_BaselineStart",&DD_BaselineStart);
    treepointers[i]->SetBranchAddress("DD_BaselineEnd",&DD_BaselineEnd);
    treepointers[i]->SetBranchAddress("DD_Ampl",&DD_Ampl);
    treepointers[i]->SetBranchAddress("DD_AmplADU",&DD_AmplADU);

    treepointers[i]->SetBranchAddress("DD_Rise",&DD_Rise);
    treepointers[i]->SetBranchAddress("DD_Rise10pct",&DD_Rise10pct);
    treepointers[i]->SetBranchAddress("DD_Rise25pct",&DD_Rise25pct);
    treepointers[i]->SetBranchAddress("DD_Rise75pct",&DD_Rise75pct);
    treepointers[i]->SetBranchAddress("DD_Rise90pct",&DD_Rise90pct);
/*
        cout << Energy[0].size()<< endl;
        cout << DD_RawRise[0].size() << endl; 

        cout << Energy[1].size()<< endl;
        cout << DD_RawRise[1].size() << endl;

        cout << Energy[2].size()<< endl;
        cout << DD_RawRise[2].size() << endl;  
*/

    if (i == 1) {

        for (int j = 0; j < treepointers[i]->GetEntries(); j++) {

            treepointers[i]->GetEntry(j); 

            sample_plot->Fill(DD_AmplADU[0][0],DD_RawRise[0][0]); 

        }

        sample_plot->SetStats(kFALSE);
        sample_plot->SetOption("colz");
        sample_plot->GetXaxis()->SetTitle("Amplitude (ADU)"); 
        sample_plot->GetYaxis()->SetTitle("Rise Time (#mus)");
        sample_plot->Write(); 

    }

    for (int j = 0; j < treepointers[i]->GetEntries(); j++) {
    
        treepointers[i]->GetEntry(j); 

        if (i==1) {
            if (ProcessId[0][0] != 301) {
                histopointers_cuts[1]->Fill(DD_Ampl[0][0]/123214,DD_RawRise[0][0]);
            }

            if (ProcessId[0][0] == 202) {
                histopointers_cuts[2]->Fill(DD_Ampl[0][0]/123214,DD_RawRise[0][0]);
            }

            if (ProcessId[0][0] == 303) {
                histopointers_cuts[3]->Fill(DD_Ampl[0][0]/123214,DD_RawRise[0][0]);
            }

            histopointers_cuts[0]->Fill(DD_Ampl[0][0]/123214,DD_RawRise[0][0]);
        }

        // main filler
        if (ProcessId[0][0] == 101) {

            histopointers[i]->Fill(DD_Ampl[0][0]/123214,DD_RawRise[0][0]); // Alexis simulated the Ar37 calibration and found 1keV = 123214 DD_Ampl units
            //histopointers[i]->Fill(DD_RawRise[0][1],DD_AmplADU[0][1]);

        }

        //cout << "Rise time " << DD_RawRise[0][1] << endl;
        //cout << "Amplitude " << DD_AmplADU[0][1] << endl; 
    
    }
    
    //histopointers[i]->GetXaxis()->SetTitle("Amplitude (ADU)");
    histopointers[i]->GetXaxis()->SetTitle("Energy (keV)");
    histopointers[i]->GetYaxis()->SetTitle("Rise Time (#mus)");

    gen_canvas->cd(i+1); 
    gen_canvas->cd(i+1)->SetLogz(); 

    histopointers[i]->Scale(1/histopointers[i]->GetMaximum()); 
    histopointers[i]->SetMaximum(0.1);
    histopointers[i]->SetMinimum(0.0001);
    //histopointers[i]->SetMaximum(0.01);
    gStyle->SetOptStat("enm");
    
    
    histopointers[i]->DrawCopy("colz");  

    //histopointers[i]->GetXaxis()->SetLabelOffset(999);
    //histopointers[i]->GetXaxis()->SetLabelSize(0);


//----------------------------------------------------------------------------------//

    if (i==1) {

        source_comparison_canvas->cd(1); 
        source_comparison_canvas->cd(1)->SetLogz(); 
        

            histopointers[i]->DrawCopy("colz"); 

            proj_ampl_ambe = histopointers[i]->ProjectionX();
            proj_rt_ambe = histopointers[i]->ProjectionY(); 

        for (int c=1; c<16; c=c+3) {

            ampl_comparison_canvas->cd(c); 
            ampl_comparison_canvas->cd(c)->SetLogz();

            if (c == 13) {
                ampl_ambe_clones[0] = (TH2F*)histopointers[i]->Clone("cl1");
                ampl_ambe_clones[0]->GetYaxis()->SetRangeUser(0,50);
                ampl_ambe_clones[0]->Draw("col"); 
            }
            else if (c == 10) {
                ampl_ambe_clones[1] = (TH2F*)histopointers[i]->Clone("cl2");
                ampl_ambe_clones[1]->GetYaxis()->SetRangeUser(50,100);
                ampl_ambe_clones[1]->Draw("col"); 
            }
            else if (c == 7) {
                ampl_ambe_clones[2] = (TH2F*)histopointers[i]->Clone("cl3");
                ampl_ambe_clones[2]->GetYaxis()->SetRangeUser(100,150);
                ampl_ambe_clones[2]->Draw("col"); 
            }
            else if (c == 4) {
                ampl_ambe_clones[3] = (TH2F*)histopointers[i]->Clone("cl4");
                ampl_ambe_clones[3]->GetYaxis()->SetRangeUser(150,200);
                ampl_ambe_clones[3]->Draw("col"); 
            }
            else if (c == 1) {
                ampl_ambe_clones[4] = (TH2F*)histopointers[i]->Clone("cl5");
                ampl_ambe_clones[4]->GetYaxis()->SetRangeUser(200,250);
                ampl_ambe_clones[4]->Draw("col"); 
            }
        }   

    }

    if (i==4) {

        source_comparison_canvas->cd(3);
        source_comparison_canvas->cd(3)->SetLogz(); 

            histopointers[i]->DrawCopy("colz"); 

            proj_ampl_dd = histopointers[i]->ProjectionX();
            proj_rt_dd = histopointers[i]->ProjectionY(); 

        for (int c=2; c<16; c=c+3) {

            ampl_comparison_canvas->cd(c); 
            ampl_comparison_canvas->cd(c)->SetLogz(); 

            if (c == 14) {
                ampl_dd_clones[0] = (TH2F*)histopointers[i]->Clone("dcl1");
                ampl_dd_clones[0]->GetYaxis()->SetRangeUser(0,50);
                ampl_dd_clones[0]->Draw("col"); 
            }
            else if (c == 11) {
                ampl_dd_clones[1] = (TH2F*)histopointers[i]->Clone("dcl2");
                ampl_dd_clones[1]->GetYaxis()->SetRangeUser(50,100);
                ampl_dd_clones[1]->Draw("col"); 
            }
            else if (c == 8) {
                ampl_dd_clones[2] = (TH2F*)histopointers[i]->Clone("dcl3");
                ampl_dd_clones[2]->GetYaxis()->SetRangeUser(100,150);
                ampl_dd_clones[2]->Draw("col"); 
            }
            else if (c == 5) {
                ampl_dd_clones[3] = (TH2F*)histopointers[i]->Clone("dcl4");
                ampl_dd_clones[3]->GetYaxis()->SetRangeUser(150,200);
                ampl_dd_clones[3]->Draw("col"); 
            }
            else if (c == 2) {
                ampl_dd_clones[4] = (TH2F*)histopointers[i]->Clone("dcl5");
                ampl_dd_clones[4]->GetYaxis()->SetRangeUser(200,250);
                ampl_dd_clones[4]->Draw("col"); 
            }
        } 
        

    }



}


//----------------------------------------------------------------------------------//
    double histocuts_max_scale = 1/histopointers_cuts[0]->GetMaximum(); 

    for (int i = 0; i <nbcuts; i++) {
        histopointers_cuts[i]->GetXaxis()->SetTitle("Energy (keV)");
        histopointers_cuts[i]->GetYaxis()->SetTitle("Rise Time (#mus)");

        histopointers_cuts[i]->Scale(histocuts_max_scale); 

        //histopointers_cuts[i]->Scale(1/histopointers_cuts[i]->GetMaximum()); 
        //histopointers_cuts[i]->SetMaximum();

        if (i!=0) {
            histopointers_cuts_proj[i] = (TH1D*)histopointers_cuts[i]->ProjectionX(); 
            histopointers_cuts_proj[i]->GetYaxis()->SetRangeUser(0.001,10); 
        }

        

        gen_canvas_cuts->cd(2*i+1);
        gen_canvas_cuts->cd(2*i+1)->SetLogz();
            histopointers_cuts[i]->SetMaximum(0.1); 
            histopointers_cuts[i]->SetMinimum(0.0001);  
            histopointers_cuts[i]->Draw("colz"); 
            histopointers_cuts[i]->SetStats(kFALSE);
            

        gen_canvas_cuts->cd(2*i+2);
        gen_canvas_cuts->cd(2*i+2)->SetLogy();   
     
    }

        histopointers_cuts_proj[0]= (TH1D*)histopointers_cuts[0]->ProjectionX(); 
        gen_canvas_cuts->cd(2);
            histopointers_cuts_proj[0]->SetLineColorAlpha(1,1);
            //histopointers_cuts_proj[0]->Draw("hist C");
            histopointers_cuts_proj[0]->SetStats(kFALSE);

        gen_canvas_cuts->cd(4); 
            histopointers_cuts_proj[1]->Draw("hist C"); 
            histopointers_cuts_proj[1]->SetStats(kFALSE);

        gen_canvas_cuts->cd(6); 
            histopointers_cuts_proj[2]->Draw("hist C");
            histopointers_cuts_proj[2]->SetStats(kFALSE);

         gen_canvas_cuts->cd(8); 
            histopointers_cuts_proj[3]->Draw("hist C");  
            histopointers_cuts_proj[3]->SetStats(kFALSE);
        
        
        gen_canvas_cuts->cd(2); 

        histopointers_cuts_proj[1]->SetLineColorAlpha(2,1);
        histopointers_cuts_proj[1]->Draw("hist C"); 
        histopointers_cuts_proj[2]->SetLineColorAlpha(3,1);
        histopointers_cuts_proj[2]->Draw("hist C same");
        histopointers_cuts_proj[3]->SetLineColorAlpha(4,1);
        histopointers_cuts_proj[3]->Draw("hist C same");

        TLegend* leg_cut = new TLegend(0.57, 0.57, .89, .89);
            //leg_cut->AddEntry(histopointers_cuts_proj[0], "Total Events", "l");
            leg_cut->AddEntry(histopointers_cuts_proj[1], "Not Elastic Events", "l");
            leg_cut->AddEntry(histopointers_cuts_proj[2], "e- ionization", "l");
            leg_cut->AddEntry(histopointers_cuts_proj[3], "Neutron Inelastic", "l");

        leg_cut->Draw();     

    for (int i = 0; i<nbcuts; i++) {
        //histopointers_cuts_proj[i]->SetStats(kFALSE); 
    }



//----------------------------------------------------------------------------------//




    for (int i = 3; i < 16; i=i+3) {
        ampl_comparison_canvas->cd(i);
        ampl_comparison_canvas->cd(i)->SetLogy(); 

        if (i == 15) {
            int y = 0; 
            proj_ampl_ambe_a[y] = ampl_ambe_clones[y]->ProjectionX(); 
            proj_ampl_dd_a[y] = ampl_dd_clones[y]->ProjectionX(); 
            proj_ampl_ambe_a[y]->SetStats(kFALSE);
            proj_ampl_ambe_a[y]->SetTitle(""); 
            proj_ampl_ambe_a[y]->Draw("hist C"); 
            proj_ampl_dd_a[y]->SetLineColorAlpha(2,1);
            proj_ampl_dd_a[y]->Draw("hist C same");
        }
        else if (i == 12) {
            int y = 1; 
            proj_ampl_ambe_a[y] = ampl_ambe_clones[y]->ProjectionX(); 
            proj_ampl_dd_a[y] = ampl_dd_clones[y]->ProjectionX(); 
            proj_ampl_ambe_a[y]->SetStats(kFALSE);
            proj_ampl_ambe_a[y]->SetTitle(""); 
            proj_ampl_ambe_a[y]->Draw("hist C"); 
            proj_ampl_dd_a[y]->SetLineColorAlpha(2,1);
            proj_ampl_dd_a[y]->Draw("hist C same");
        }
        else if (i == 9) {
            int y = 2; 
            proj_ampl_ambe_a[y] = ampl_ambe_clones[y]->ProjectionX(); 
            proj_ampl_dd_a[y] = ampl_dd_clones[y]->ProjectionX(); 
            proj_ampl_ambe_a[y]->SetStats(kFALSE);
            proj_ampl_ambe_a[y]->SetTitle(""); 
            proj_ampl_ambe_a[y]->Draw("hist C"); 
            proj_ampl_dd_a[y]->SetLineColorAlpha(2,1);
            proj_ampl_dd_a[y]->Draw("hist C same");
        }
        else if (i == 6) {
            int y = 3; 
            proj_ampl_ambe_a[y] = ampl_ambe_clones[y]->ProjectionX(); 
            proj_ampl_dd_a[y] = ampl_dd_clones[y]->ProjectionX(); 
            proj_ampl_ambe_a[y]->SetStats(kFALSE);
            proj_ampl_ambe_a[y]->SetTitle(""); 
            proj_ampl_ambe_a[y]->Draw("hist C"); 
            proj_ampl_dd_a[y]->SetLineColorAlpha(2,1);
            proj_ampl_dd_a[y]->Draw("hist C same");
        }
        else if (i == 3) {
            int y = 4; 
            proj_ampl_ambe_a[y] = ampl_ambe_clones[y]->ProjectionX(); 
            proj_ampl_dd_a[y] = ampl_dd_clones[y]->ProjectionX(); 
            proj_ampl_ambe_a[y]->SetStats(kFALSE);
            proj_ampl_ambe_a[y]->SetTitle(""); 
            proj_ampl_ambe_a[y]->Draw("hist C"); 
            proj_ampl_dd_a[y]->SetLineColorAlpha(2,1);
            proj_ampl_dd_a[y]->Draw("hist C same");
        }
    
    }

    for (int k= 0; k<5; k++) {
        ampl_ambe_clones[k]->SetStats(kFALSE); 
        ampl_dd_clones[k]->SetStats(kFALSE); 
        ampl_ambe_clones[k]->GetXaxis()->SetLabelOffset(999);
        ampl_ambe_clones[k]->GetXaxis()->SetLabelSize(0);
        ampl_dd_clones[k]->GetXaxis()->SetLabelOffset(999);
        ampl_dd_clones[k]->GetXaxis()->SetLabelSize(0);
    }




//----------------------------------------------------------------------------------//

    TLegend* leg_ampl = new TLegend(0.65, 0.65, .85, .85);
    leg_ampl->AddEntry(proj_ampl_ambe, "AmBe", "l");
    leg_ampl->AddEntry(proj_ampl_dd, "DD", "l");

    TLegend* leg_rt = new TLegend(0.65, 0.65, .85, .85);
    leg_rt->AddEntry(proj_rt_ambe, "AmBe", "l");
    leg_rt->AddEntry(proj_rt_dd, "DD", "l");



    source_comparison_canvas->cd(2);
    source_comparison_canvas->cd(2)->SetLogy();

    proj_ampl_ambe->SetTitle("Energy Projection");
    proj_ampl_ambe->SetStats(kFALSE); 
    proj_ampl_ambe->Draw("hist C"); 
    proj_ampl_dd->SetLineColorAlpha(2,1);
    proj_ampl_dd->Draw("hist C same"); 
    leg_ampl->Draw(); 

    source_comparison_canvas->cd(4); 
    source_comparison_canvas->cd(4)->SetLogy();

    proj_rt_ambe->SetTitle("Risetime Projection");
    proj_rt_ambe->SetStats(kFALSE); 
    proj_rt_ambe->Draw("hist C"); 
    proj_rt_dd->SetLineColorAlpha(2,1);
    proj_rt_dd->Draw("hist C same"); 
    leg_rt->Draw(); 



    





//----------------------------------------------------------------------------------//


return 0; 
}
