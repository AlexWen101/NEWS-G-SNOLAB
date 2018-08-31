#ifndef __SHIELD_PRIMARY_GENERATOR_ACTION_H_
#define __SHIELD_PRIMARY_GENERATOR_ACTION_H_

#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleGun.hh>
#include <G4Event.hh>
#include <G4Point3D.hh>
#include <G4Vector3D.hh>
#include <G4Types.hh>

#include "detector_construction.h"
#include "decay.h"
#include "run_type.h"

namespace shield {

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
	
public:
	
	PrimaryGeneratorAction();
	virtual ~PrimaryGeneratorAction();
	
	void CreateDecays();
	void DestroyDecays();
	virtual void GeneratePrimaries(G4Event* event);
	
private:
	
	std::vector<G4ParticleGun*> m_guns;
	std::vector<Decay> m_decays;
	DetectorConstruction const* m_detectorConstruction;
	RunType m_runType;
};

}

#endif

