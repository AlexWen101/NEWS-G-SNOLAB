#include "detector_construction.h"
#include "action_initialization.h"
#include "run_type.h"

#include <G4RunManager.hh>
#include <G4MTRunManager.hh>
#include <Shielding.hh>
#include <G4UImanager.hh>
#include <G4UIExecutive.hh>
#include <G4SystemOfUnits.hh>
#include <G4Types.hh>

#ifdef SHIELD_VIS_ENABLE
#include <G4VisManager.hh>
#include <G4VisExecutive.hh>
#endif

#include <CLHEP/Random/Randomize.h>

#include <ctime>
#include <sstream>

using namespace shield;

int main(int argc, char** argv) {
	G4MTRunManager* runManager = new G4MTRunManager();
	G4UImanager* uiManager = G4UImanager::GetUIpointer();
	#ifdef SHIELD_VIS_ENABLE
	G4VisManager* visManager = new G4VisExecutive();
	#endif
	G4UIExecutive* ui = new G4UIExecutive(argc, argv);
	
	CLHEP::HepRandomEngine* randomEngine = new CLHEP::HepJamesRandom();
	int seed = std::time(NULL);
	randomEngine->setSeed(seed, 3);
	CLHEP::HepRandom::setTheEngine(randomEngine);
	
	ShieldingInfo shieldingInfo;
	GloveboxInfo gloveboxInfo;
	TubeInfo tubeInfo;
	// The dimensions of the detector could be modified from the defaults
	// here. Later, this will be accessible through configure_commands.mac.
	// Example:
	// gloveboxInfo.hdpeShieldThickness += 50*mm;
	DetectorConstruction* detectorConstruction = new DetectorConstruction(
		// Size of the world.
		5*m,
		shieldingInfo,
		gloveboxInfo,
		tubeInfo);
	Shielding* physicsList = new Shielding();
	ActionInitialization* actionInitialization = new ActionInitialization(
		detectorConstruction);
	runManager->SetUserInitialization(detectorConstruction);
	runManager->SetUserInitialization(physicsList);
	runManager->SetUserInitialization(actionInitialization);
	
	#ifdef SHIELD_VIS_ENABLE
	visManager->Initialize();
	#endif
	
	uiManager->ExecuteMacroFile("configure_commands.mac");
	#ifdef SHIELD_VIS_ENABLE
	uiManager->ExecuteMacroFile("vis_commands.mac");
	#else
	uiManager->ExecuteMacroFile("run_commands.mac");
	#endif
	
	// Only keep the program running if there is visualization.
	#ifdef SHIELD_VIS_ENABLE
	ui->SessionStart();
	#endif
	
	delete ui;
	#ifdef SHIELD_VIS_ENABLE
	delete visManager;
	#endif
	delete runManager;
	
	return 0;
}

