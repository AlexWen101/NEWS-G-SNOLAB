#include "run_action.h"

#include <ctime>
#include <sstream>

#include <G4UImanager.hh>
#include <G4CsvAnalysisManager.hh>
#include <G4AccumulableManager.hh>
#include <G4SystemOfUnits.hh>

#include <unistd.h>

#include "run_type.h"

using namespace shield;

G4Run* RunAction::GenerateRun() {
	return G4UserRunAction::GenerateRun();
}


void RunAction::BeginOfRunAction(const G4Run*) {
	// Load variables of the run from the UI manager.
	G4UImanager* uiManager = G4UImanager::GetMasterUIpointer();
	G4String runName = uiManager->SolveAlias("{runName}");
	RunType runType = RunTypeFromString(uiManager->SolveAlias("{runType}"));
	
	// Reset accumulables.
	G4AccumulableManager::Instance()->Reset();
	
	// Prepare the primary generator action decay type.
	if (m_primaryGeneratorAction) {
		m_primaryGeneratorAction->CreateDecays();
	}

	if (IsMaster()) {
		
		G4CsvAnalysisManager* analysisManager = G4CsvAnalysisManager::Instance();
		std::time_t timeRaw = std::time(NULL);
		std::tm* time = std::localtime(&timeRaw);
		std::stringstream fileNameStream;
		char dateStr[128];
		std::strftime(dateStr, sizeof(dateStr), "%F_%H-%M", time); 
		
		G4String runLocation = uiManager->SolveAlias("{runLocation}");
		G4String runLocationShort = "def"; 
		if (runLocation == "origin") {runLocationShort = "Org";}
		else if (runLocation == "location1") {runLocationShort = "L1";}
		else if (runLocation == "location2") {runLocationShort = "L2";}
		else if (runLocation == "location3") {runLocationShort = "L3";}
		else if (runLocation == "location4") {runLocationShort = "L4";}
		else if (runLocation == "location5") {runLocationShort = "L5";}
		else if (runLocation == "location6") {runLocationShort = "L6";}
		
		fileNameStream <<
			//runName << "_" <<
			RunTypeToString(runType) <<
			"_" <<  
			runLocationShort << 
			"_" <<
			dateStr << 
			// "_" <<
			// ::getpid() << 
			".csv";
		
		G4String fileName = fileNameStream.str();

		analysisManager->SetFileName(fileName);

		//analysisManager->OpenFile();
		
		// Create the tuples for the data.
		
		analysisManager->CreateNtuple("Particles", "Scored particles");

		analysisManager->CreateNtupleIColumn("Event Index");

		analysisManager->CreateNtupleDColumn("Global Time (s)");

		analysisManager->CreateNtupleSColumn("Particle Name");
		analysisManager->CreateNtupleSColumn("Particle Type");
		analysisManager->CreateNtupleDColumn("Initial Particle Kinetic Energy (MeV)");
		analysisManager->CreateNtupleSColumn("Process Creator");
		analysisManager->CreateNtupleIColumn("Particle ID");
		analysisManager->CreateNtupleSColumn("Mother Name");
		analysisManager->CreateNtupleIColumn("Mother ID");

		analysisManager->CreateNtupleDColumn("Pre-Step Position x (mm)");
		analysisManager->CreateNtupleDColumn("Pre-Step Position y (mm)");
		analysisManager->CreateNtupleDColumn("Pre-Step Position z (mm)");
		analysisManager->CreateNtupleDColumn("Post-Step Position x (mm)");
		analysisManager->CreateNtupleDColumn("Post-Step Position y (mm)");
		analysisManager->CreateNtupleDColumn("Post-Step Position z (mm)");

		analysisManager->CreateNtupleDColumn("Step Ionizing Energy Deposit (MeV)");
		analysisManager->CreateNtupleDColumn("Step Total Energy (MeV)");
		analysisManager->CreateNtupleDColumn("Step Kinetic Energy (MeV)");

		analysisManager->CreateNtupleSColumn("Step Process");

		analysisManager->CreateNtupleDColumn("Initial Particle Position x (mm)");
		analysisManager->CreateNtupleDColumn("Initial Particle Position y (mm)");
		analysisManager->CreateNtupleDColumn("Initial Particle Position z (mm)");

		analysisManager->CreateNtupleSColumn("Initial Particle Name");

		analysisManager->CreateNtupleDColumn("PositionRef x (mm)");
		analysisManager->CreateNtupleDColumn("PositionRef y (mm)");
		analysisManager->CreateNtupleDColumn("PositionRef z (mm)");

		analysisManager->CreateNtupleSColumn("Particle when Time Exceeds 1 ms");

		analysisManager->CreateNtupleDColumn("PositionTime x (mm)");
		analysisManager->CreateNtupleDColumn("PositionTime y (mm)");
		analysisManager->CreateNtupleDColumn("PositionTime z (mm)");

		analysisManager->CreateNtupleIColumn("Time difference (> 1 ms) counter");

		
		analysisManager->FinishNtuple();


	
	}

	m_initialTime = std::chrono::steady_clock::now(); 

}







void RunAction::EndOfRunAction(const G4Run* run) {


// Load variables of the run from the UI manager.
G4UImanager* uiManager = G4UImanager::GetMasterUIpointer();
G4String runName = uiManager->SolveAlias("{runName}");
RunType runType = RunTypeFromString(uiManager->SolveAlias("{runType}"));
G4double sourceActivity = RunTypeGetActivity(runType);
	
G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
accumulableManager->Merge();
	
G4cout << "\n-------------RUN FINISHED---------------\n\n";
if (IsMaster()) {
	G4cout << "  Master thread finished.\n";
	}
else {
		G4cout << "  Local thread finished.\n";
	}
	
G4int numEvents = run->GetNumberOfEvent();
G4int numScores = m_particleScores->Size();
std::chrono::duration<double> programDuration =
	std::chrono::steady_clock::now() - m_initialTime;
G4double simulatedTime = numEvents / sourceActivity;
	
G4cout << "  Program time was " << programDuration.count() << " s.\n";
G4cout << "  Simulated time was " << simulatedTime/s << " s.\n";
G4cout << "  Number of events was " << numEvents << ".\n";
G4cout << "  Number of particle tracks was " << numScores << ".\n";
G4cout << "  Have a nice day!\n";
	
G4cout << "\n----------------------------------------\n\n";
	
G4CsvAnalysisManager* analysisManager = G4CsvAnalysisManager::Instance();

//G4CsvAnalysisManager* analysisManager = G4CsvAnalysisManager::Instance();
		
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
/*
		std::time_t timeRaw = std::time(NULL);
		std::tm* time = std::localtime(&timeRaw);
		std::stringstream fileNameStream;
		char dateStr[128];
		std::strftime(dateStr, sizeof(dateStr), "%F_%H-%M", time); 
		
		G4String runLocation = uiManager->SolveAlias("{runLocation}");
		G4String runLocationShort = "def"; 
		if (runLocation == "origin") {runLocationShort = "O";}
		else if (runLocation == "location1") {runLocationShort = "L1";}
		else if (runLocation == "location2") {runLocationShort = "L2";}
		else if (runLocation == "location3") {runLocationShort = "L3";}
		else if (runLocation == "location4") {runLocationShort = "L4";}
		else if (runLocation == "location5") {runLocationShort = "L5";}
		
		fileNameStream <<
			//runName << "_" <<
			RunTypeToString(runType) <<
			"_" <<  
			runLocationShort << 
			"_" <<
			dateStr << 
			// "_" <<
			// ::getpid() << 
			".csv";
		G4String fileName = fileNameStream.str();
*/
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

	

	if (IsMaster()) {

		analysisManager->OpenFile(); 

		for (
				auto it = m_particleScores->Begin();
				it != m_particleScores->End();
				++it
			) 
			
		{
			ParticleScore particleScore = *it;

			analysisManager->FillNtupleIColumn(0,particleScore.eventId);

			analysisManager->FillNtupleDColumn(1,particleScore.globalTime/s);
			
			analysisManager->FillNtupleSColumn(2,particleScore.particleName);
			analysisManager->FillNtupleSColumn(3,particleScore.particleType);
			analysisManager->FillNtupleDColumn(4,particleScore.initialEkin/MeV);
			analysisManager->FillNtupleSColumn(5,particleScore.processCreator);
			analysisManager->FillNtupleIColumn(6,particleScore.particleId);
			analysisManager->FillNtupleSColumn(7,particleScore.motherName);
			analysisManager->FillNtupleIColumn(8,particleScore.motherId);

			G4Point3D positionPre = particleScore.positionPre;
			analysisManager->FillNtupleDColumn(9,positionPre.x()/mm);
			analysisManager->FillNtupleDColumn(10,positionPre.y()/mm);
			analysisManager->FillNtupleDColumn(11,positionPre.z()/mm);

			G4Point3D positionPost = particleScore.positionPost;
			analysisManager->FillNtupleDColumn(12,positionPost.x()/mm);
			analysisManager->FillNtupleDColumn(13,positionPost.y()/mm);
			analysisManager->FillNtupleDColumn(14,positionPost.z()/mm);

			analysisManager->FillNtupleDColumn(15,particleScore.stepEnergyDifference/MeV);
			analysisManager->FillNtupleDColumn(16,particleScore.stepTotalEnergy/MeV);
			analysisManager->FillNtupleDColumn(17,particleScore.stepKineticEnergy/MeV);

			analysisManager->FillNtupleSColumn(18,particleScore.process);

			G4Point3D positionInitial = particleScore.positionInitial;
			analysisManager->FillNtupleDColumn(19,positionInitial.x()/mm);
			analysisManager->FillNtupleDColumn(20,positionInitial.y()/mm);
			analysisManager->FillNtupleDColumn(21,positionInitial.z()/mm);

			analysisManager->FillNtupleSColumn(22,particleScore.refPartName);

			G4Point3D positionRef = particleScore.positionRef;
			analysisManager->FillNtupleDColumn(23,positionRef.x()/mm);
			analysisManager->FillNtupleDColumn(24,positionRef.y()/mm);
			analysisManager->FillNtupleDColumn(25,positionRef.z()/mm);

			analysisManager->FillNtupleSColumn(26,particleScore.timeParticle);

			G4Point3D positionTime = particleScore.positionTime; 
			analysisManager->FillNtupleDColumn(27,positionTime.x()/mm);
			analysisManager->FillNtupleDColumn(28,positionTime.y()/mm);
			analysisManager->FillNtupleDColumn(29,positionTime.z()/mm);

			analysisManager->FillNtupleIColumn(30,particleScore.subEventNb);
			
			analysisManager->AddNtupleRow();

			/*
			analysisManager->FillNtupleIColumn(0, 0, particleScore.eventIndex);
			//analysisManager->FillNtupleIColumn(0, 1, particleScore.parentIndex);
			//analysisManager->FillNtupleIColumn(0, 2, particleScore.trackIndex);
			//analysisManager->FillNtupleSColumn(0, 3, particleScore.logicalVolume);
			analysisManager->FillNtupleSColumn(0, 1, particleScore.process);
			analysisManager->FillNtupleSColumn(0, 2, particleScore.particle);

			analysisManager->FillNtupleIColumn(0, 3, particleScore.stepNumber);
			
			//analysisManager->FillNtupleDColumn(0, 6, particleScore.time/us);
			
			G4Point3D position = particleScore.position;
			analysisManager->FillNtupleDColumn(0, 4, positionPre.x()/mm);
			analysisManager->FillNtupleDColumn(0, 5, positionPre.y()/mm);
			analysisManager->FillNtupleDColumn(0, 6, positionPre.z()/mm);
			
			//G4Vector3D direction = particleScore.direction;
			//analysisManager->FillNtupleDColumn(0, 10, direction.x());
			//analysisManager->FillNtupleDColumn(0, 11, direction.y());
			//analysisManager->FillNtupleDColumn(0, 12, direction.z());
			
			analysisManager->FillNtupleDColumn(0, 7, particleScore.energyDeposit/MeV);
			analysisManager->FillNtupleDColumn(0, 8, particleScore.energy/MeV);
			analysisManager->AddNtupleRow(0);
			*/
		}
		
		

		analysisManager->Write();
	    analysisManager->CloseFile();
		
	}

	
	//m_initialTime = std::chrono::steady_clock::now();
	
}


