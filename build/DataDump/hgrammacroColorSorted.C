//#include "TFile.h"
//#include "TTree.h"
//#include "TH2F.h"
//#include "TString.h"
//#include "TCanvas.h"

void macro() {

    const int nbFiles = 4;

    TString filenames[nbFiles]={
        "Run_AmBeNeutron_2018-05-24_090250_994_nt_Particles.root",
        "Run_DDNeutron_2018-05-24_093759_1642_nt_Particles.root",
        "Run_AmBeNeutron_2018-05-24_083059_722_nt_Particles.root",
        "Run_DDNeutron_2018-05-24_100320_1945_nt_Particles.root"
    };

    TString h[nbFiles]={"h1", "h2", "h3", "h4"};

    TString hnames[nbFiles]={"pos1-ambe","pos1-dd","pos2-ambe","pos2-dd"};

    TCanvas *c1 = new TCanvas("particle-energydep","particle-energydep",1100,900);
    c1->Divide(2,2);

    Int_t bins = 800;
    Float_t upperspacing = 0.01;
    Float_t lowerspacing = 0;

    

    TH2F *hist[nbFiles];
    for (Int_t i=0; i<nbFiles; i++){
      hist[i]= new TH2F(hnames[i],hnames[i],
      bins,lowerspacing,upperspacing,
      bins,lowerspacing,upperspacing);
    }

    for (Int_t i=0; i<nbFiles; i++){

        TFile *f = new TFile(filenames[i]);

        //printf("Hello!");

        TTree *ntuple = (TTree*)f->Get("ntuple");

        //printf("Hello2!");

        Int_t EventIndex, ParentIndex, TrackIndex;
        char *LogicalVolume[50], *ProcessName[50], *ParticleName[50];
        Float_t ParticleTime, XPosition, YPosition, ZPosition, XDirection, YDirection, ZDirection, EnergyDeposit, Energy;

        ntuple->SetBranchAddress("EventIndex",&EventIndex);
        ntuple->SetBranchAddress("ParentIndex",&ParentIndex);
        ntuple->SetBranchAddress("TrackIndex",&TrackIndex);

        ntuple->SetBranchAddress("LogicalVolume",LogicalVolume);
        ntuple->SetBranchAddress("ProcessName",ProcessName);
        ntuple->SetBranchAddress("ParticleName",ParticleName);

        ntuple->SetBranchAddress("ParticleTime",&ParticleTime);

        ntuple->SetBranchAddress("XPosition",&XPosition);
        ntuple->SetBranchAddress("YPosition",&YPosition);
        ntuple->SetBranchAddress("ZPosition",&ZPosition);

        ntuple->SetBranchAddress("XDirection",&XDirection);
        ntuple->SetBranchAddress("YDirection",&YDirection);
        ntuple->SetBranchAddress("ZDirection",&ZDirection);

        ntuple->SetBranchAddress("EnergyDeposit",&EnergyDeposit);
        ntuple->SetBranchAddress("Energy",&Energy);

        //printf("Hello3!");

        c1->cd(i+1);

        Int_t nentries = (Int_t)ntuple->GetEntries();

        //printf("Hello!");
        //printf("%d\n",nentries);
       // printf("%f\n",ParticleTime);
        //printf("%s\n",ProcessName);

        for (Int_t j=0; j<nentries; j++){
            ntuple->GetEntry(j);
            //hist[i]->Fill(Energy,EnergyDeposit);
        }

        ntuple->Draw("Energy:EnergyDeposit>>histogram","","goff");
        hist[i]->Draw();

        c1->Update();
        /*
        ntuple->SetMarkerColor(2); ntuple->Draw("EnergyDeposit:Energy","XPosition<0","same");
        ntuple->SetMarkerColor(3); ntuple->Draw("EnergyDeposit:Energy","XPosition>0","same");
        */
        
        ntuple->SetMarkerColor(2); ntuple->Draw("EnergyDeposit:Energy","ProcessName==\"neutronInelastic\"","same");
        ntuple->SetMarkerColor(3); ntuple->Draw("EnergyDeposit:Energy","ProcessName==\"RadioactiveDecay\"","same");
        ntuple->SetMarkerColor(4); ntuple->Draw("EnergyDeposit:Energy","ProcessName==\"eIoni\"","same");
        
        /*
        ntuple->SetMarkerColor(2); ntuple->Draw("EnergyDeposit:Energy","ParticleName==\"neutron\"","same");
        ntuple->SetMarkerColor(3); ntuple->Draw("EnergyDeposit:Energy","ParticleName==\"gamma\"","same");
        ntuple->SetMarkerColor(4); ntuple->Draw("EnergyDeposit:Energy","ParticleName==\"e-\"","same");
        ntuple->SetMarkerColor(5); ntuple->Draw("EnergyDeposit:Energy","ParticleName==\"Ne20\"","same");
        */
        
        
        /*
        TPaveText *pt = new TPaveText(0.5,0.5,0.5,0.5);
        pt->AddText("WHAT?");

        pt->Draw();
        
        
        auto *text = new TText(0.33,0.165,"Edep vs E by PARTICLE");
        text->SetTextColor(2); text->SetTextSize(10);
        text->Draw();
        */
       

        //printf("Hello! WHY WONT THIS WORK DAMMIT");
        

    }
   //c1->Draw();
   //for (Int_t i=0; i<nbFiles; i++){
    // delete hist[i];
   //}
}