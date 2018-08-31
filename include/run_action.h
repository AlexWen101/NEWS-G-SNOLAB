#ifndef __SHIELD_RUN_ACTION_H_
#define __SHIELD_RUN_ACTION_H_

#include <chrono>
#include <string>

#include <G4UserRunAction.hh>
#include <G4Run.hh>
#include <G4Types.hh>

#include "particle_score.h"
#include "primary_generator_action.h"
#include "accumulable_list.h"

namespace shield {

class RunAction : public G4UserRunAction {
	
public:
	
	RunAction(
		AccumulableList<ParticleScore>* particleScores,
		PrimaryGeneratorAction* primaryGeneratorAction) :
		m_particleScores(particleScores),
		m_primaryGeneratorAction(primaryGeneratorAction) {
	}
	virtual ~RunAction() {
	}
	
	virtual G4Run* GenerateRun();
	virtual void BeginOfRunAction(const G4Run * run);
	virtual void EndOfRunAction(const G4Run * run);
	
private:
	
	std::chrono::time_point<std::chrono::steady_clock> m_initialTime;
	AccumulableList<ParticleScore>* m_particleScores;
	PrimaryGeneratorAction* m_primaryGeneratorAction;
};

}

#endif

