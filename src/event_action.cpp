#include "event_action.h"

using namespace shield;

void EventAction::BeginOfEventAction(G4Event const* event) {
	m_eventParticleScores->Reset();
	*m_enteredScoringVolume = false;
}

void EventAction::EndOfEventAction(G4Event const* event) {
	if (*m_enteredScoringVolume) {
		m_particleScores->Merge(*m_eventParticleScores);
	}
	
	m_eventParticleScores->Reset();
	*m_enteredScoringVolume = false;
}

