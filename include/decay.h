#ifndef __SHIELD_DECAY_H_
#define __SHIELD_DECAY_H_

#include <G4Types.hh>

#include "spectrum.h"

namespace shield {

struct Decay {
	G4String particle;
	G4int numParticles;
	G4double frequency;
	Spectrum spectrum;
};

}

#endif

