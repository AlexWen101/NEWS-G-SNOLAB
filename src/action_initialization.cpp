#include "action_initialization.h"

#include <G4RunManager.hh>
#include <G4UImanager.hh>
#include <G4AccumulableManager.hh>
#include <G4Accumulable.hh>
#include <G4MergeMode.hh>
#include <G4LogicalVolume.hh>
#include <G4Point3D.hh>
#include <G4Types.hh>
#include <G4SystemOfUnits.hh>

#include "primary_generator_action.h"
#include "run_action.h"
#include "event_action.h"
#include "stepping_action.h"

#include "accumulable_list.h"
#include "particle_score.h"
#include "decay.h"

using namespace shield;

void ActionInitialization::BuildForMaster() const {
	G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
	AccumulableList<ParticleScore>* particleScores =
		new AccumulableList<ParticleScore>();
	accumulableManager->RegisterAccumulable(particleScores);
	SetUserAction(new RunAction(particleScores, NULL));
}

void ActionInitialization::Build() const {
	// The accumulables store how much energy has been deposited into the
	// detector by different kinds of particles.
	G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
	AccumulableList<ParticleScore>* particleScores =
		new AccumulableList<ParticleScore>();
	AccumulableList<ParticleScore>* eventParticleScores =
		new AccumulableList<ParticleScore>();
	accumulableManager->RegisterAccumulable(particleScores);
	
	bool* enteredScoringVolume = new bool(false);
	
	G4Point3D position = m_detectorConstruction->GetSourcePosition();
	
	// Set the event actions.
	PrimaryGeneratorAction* primaryGeneratorAction =
		new PrimaryGeneratorAction();
	SetUserAction(primaryGeneratorAction);
	SetUserAction(new RunAction(
		particleScores,
		primaryGeneratorAction));
	SetUserAction(new EventAction(
		particleScores,
		eventParticleScores,
		enteredScoringVolume));
	SetUserAction(new SteppingAction(
		m_detectorConstruction,
		eventParticleScores,
		enteredScoringVolume));
}

