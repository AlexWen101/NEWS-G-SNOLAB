#ifndef __SHIELD_EVENT_ACTION_H_
#define __SHIELD_EVENT_ACTION_H_

#include <G4UserEventAction.hh>

#include "accumulable_list.h"
#include "particle_score.h"

namespace shield {

class EventAction : public G4UserEventAction {
	
public:
	
	EventAction(
		AccumulableList<ParticleScore>* particleScores,
		AccumulableList<ParticleScore>* eventParticleScores,
		bool* enteredScoringVolume) :
		m_particleScores(particleScores),
		m_eventParticleScores(eventParticleScores),
		m_enteredScoringVolume(enteredScoringVolume) {
	}
	virtual ~EventAction() {
	}
	
	virtual void BeginOfEventAction(G4Event const* event);
	virtual void EndOfEventAction(G4Event const* event);
	
private:
	
	AccumulableList<ParticleScore>* m_particleScores;
	AccumulableList<ParticleScore>* m_eventParticleScores;
	bool* m_enteredScoringVolume;
};

}

#endif

