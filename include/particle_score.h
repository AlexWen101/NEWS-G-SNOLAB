#ifndef __SHIELD_PARTICLE_SCORE_H_
#define __SHIELD_PARTICLE_SCORE_H_

#include <G4Point3D.hh>
#include <G4Vector3D.hh>
#include <G4Types.hh>

namespace shield {

struct ParticleScore {

	G4int eventId;
	G4double globalTime;
	G4String particleName;
	G4String particleType;
	G4double initialEkin;
	G4String processCreator;
	G4int particleId;
	G4String motherName;
	G4int motherId;
	G4Point3D positionPre;
	G4Point3D positionPost;
	G4double stepEnergyDifference;
	G4double stepTotalEnergy;
	G4double stepKineticEnergy;
	G4String process;
	G4Point3D positionInitial;
	G4String refPartName;
	G4Point3D positionRef;
	G4String timeParticle;
	G4Point3D positionTime;
	G4int subEventNb;

};

}

#endif

