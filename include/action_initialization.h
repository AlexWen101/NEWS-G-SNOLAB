#ifndef __SHIELD_ACTION_INITIALIZATION_H_
#define __SHIELD_ACTION_INITIALIZATION_H_

#include <G4VUserActionInitialization.hh>
#include <G4Types.hh>

#include "detector_construction.h"
#include "run_type.h"

namespace shield {

class ActionInitialization : public G4VUserActionInitialization {
	
public:
	
	ActionInitialization(DetectorConstruction* detectorConstruction) :
		m_detectorConstruction(detectorConstruction) {
	}
	virtual ~ActionInitialization() {
	}
	
	virtual void BuildForMaster() const;
	virtual void Build() const;
	
private:
	
	DetectorConstruction* m_detectorConstruction;
};

}

#endif

