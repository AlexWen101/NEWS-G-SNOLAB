//#include "TFile.h"
//#include "TTree.h"
//#include "TH2F.h"
//#include "TString.h"
//#include "TCanvas.h"

void hgrammacro() {

    const int nbFiles = 4;

    TString filenames[nbFiles]={
        "Run_AmBeNeutron_2018-05-24_090250_994_nt_Particles.root",
        "Run_DDNeutron_2018-05-24_093759_1642_nt_Particles.root",
        "Run_AmBeNeutron_2018-05-24_083059_722_nt_Particles.root",
        "Run_DDNeutron_2018-05-24_100320_1945_nt_Particles.root"
    };

    TString h[nbFiles]={"h1", "h2", "h3", "h4"};

    TString hnames[nbFiles]={"pos1-ambe","pos1-dd","pos2-ambe","pos2-dd"};

    TCanvas *c1 = new TCanvas("particle-energydep","particle-energydep",900,900);
    c1->Divide(2,2);

    Int_t bins = 800;
    Float_t upperspacing = 0.04;
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
            hist[i]->Fill(Energy,EnergyDeposit);
        }

        hist[i]->Draw(" ");

        //printf("Hello! WHY WONT THIS WORK DAMMIT");
        

    }
   //c1->Draw();
   //for (Int_t i=0; i<nbFiles; i++){
    // delete hist[i];
   //}
}