#ifndef __SHIELD_STEPPING_ACTION_H_
#define __SHIELD_STEPPING_ACTION_H_

#include <G4UserSteppingAction.hh>
#include <G4Step.hh>
#include <G4Accumulable.hh>
#include <G4Types.hh>
#include <G4Track.hh>
#include "G4ThreeVector.hh"
#include "G4Threading.hh"

#include "detector_construction.h"

#include "accumulable_list.h"
#include "particle_score.h"

#include <vector>



namespace shield {

class SteppingAction : public G4UserSteppingAction {
	
public:
	
	SteppingAction(
		DetectorConstruction* detectorConstruction,
		AccumulableList<ParticleScore>* eventParticleScores,
		bool* enteredScoringVolume) :
		m_detectorConstruction(detectorConstruction),
		m_eventParticleScores(eventParticleScores),
		m_enteredScoringVolume(enteredScoringVolume) {
	}
	virtual ~SteppingAction() {
	}
	
	virtual void UserSteppingAction(G4Step const* step);
	
private:
	
	DetectorConstruction* m_detectorConstruction;
	AccumulableList<ParticleScore>* m_eventParticleScores;
	bool* m_enteredScoringVolume;


	G4LogicalVolume* scoringVolume;

/*
	std::vector<G4String>	sa_VecPartName;
std::vector<G4int>		sa_VecPartId;

G4String motherName;
G4String refPartName;
G4String timeParticle;
G4String processCreator;
G4String process;
G4String particleName;
G4String particleType; 

G4int motherId;
G4int subEventNb;
G4int particleId;
G4int stepNumber;
	
G4Point3D positionTime;
G4Point3D positionInitial;
G4Point3D positionRef;

G4double initialEkin;
*/

};

}

#endif

