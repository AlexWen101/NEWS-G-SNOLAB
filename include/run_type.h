#ifndef __SHIELD_RUN_TYPE_H_
#define __SHIELD_RUN_TYPE_H_

#include <vector>

#include <G4String.hh>
#include <G4Types.hh>

#include "decay.h"

namespace shield {

enum class RunType {
	Unknown,
	AmBeNeutron,

	DDNeutron,

	AmBeCaptureGamma,
	AmBeGamma,
	Na22Gamma,
	BackgroundNeutron,
	// Not yet implemented
	BackgroundGamma,
};

G4String RunTypeToString(RunType runType);
RunType RunTypeFromString(G4String string);
G4double RunTypeGetActivity(RunType runType);
std::vector<Decay> RunTypeGetDecays(RunType runType);
bool RunTypeIsBackground(RunType runType);

}

#endif

