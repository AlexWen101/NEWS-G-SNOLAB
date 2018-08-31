#include "stepping_action.h"

#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>
#include <G4Track.hh>
#include <G4TrackStatus.hh>
#include <G4ParticleDefinition.hh>
#include <G4VProcess.hh>
#include <G4RunManager.hh>
#include <G4Event.hh>
#include <G4Point3D.hh>
#include <G4Vector3D.hh>
#include <G4Types.hh>
#include "G4SystemOfUnits.hh"

#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>

using namespace shield;



	G4int subEventNb = 0;

	
void SteppingAction::UserSteppingAction(G4Step const* step) {

	std::vector<G4String>	sa_VecPartName;
	std::vector<G4int>		sa_VecPartId;

	G4double initialEkin = 0.;
	G4int particleId = 0; 
	G4int motherId = 0;
	G4int stepNumber = 0;
	G4int eventId = -1;

	G4double stepEnergyDifference = 0.;
	G4double stepTotalEnergy = 0.;
	G4double stepKineticEnergy = 0.;

	G4Point3D positionInitial = G4Translate3D(0*cm,0*cm,0*cm) * G4Point3D();
	G4Point3D positionRef = G4Translate3D(0*cm,0*cm,0*cm) * G4Point3D();
	G4Point3D positionTime = G4Translate3D(0*cm,0*cm,0*cm) * G4Point3D();
	G4Point3D positionPre = G4Translate3D(0*cm,0*cm,0*cm) * G4Point3D();
	G4Point3D positionPost = G4Translate3D(0*cm,0*cm,0*cm) * G4Point3D();

	G4String refPartName = "undefined";
	G4String motherName = "undefined";
	G4String particleName = "undefined";
	G4String processCreator = "First";
	G4String timeParticle = "undefined";
	G4String particleType = "undefined";
	G4String process = "undefined";

	
	scoringVolume = m_detectorConstruction->GetScoringVolume();
	
	// Get the volume that the particle is in and check if it's the scoring
	// volume.
	G4VPhysicalVolume* prePhysicalVolume =
		step->GetPreStepPoint()->
		GetTouchableHandle()->
		GetVolume();
	
	G4LogicalVolume* preLogicalVolume = NULL;
	if (prePhysicalVolume != NULL) {
		preLogicalVolume = prePhysicalVolume->GetLogicalVolume();
	}
	
	if (preLogicalVolume == scoringVolume) {
		// Get the information about the particle.
		G4Track* track = step->GetTrack();
		G4ParticleDefinition const* particle = track->GetParticleDefinition();
		G4double globalTime = step->GetPostStepPoint()->GetGlobalTime();
		positionPre = step->GetPreStepPoint()->GetPosition();
		G4Vector3D direction = step->GetPreStepPoint()->GetMomentumDirection();
		G4double energyDeposit = step->GetTotalEnergyDeposit();
		G4double energy = step->GetPreStepPoint()->GetKineticEnergy();
		G4VProcess const* preProcessCreator = track->GetCreatorProcess();

		process = step -> GetPostStepPoint() -> GetProcessDefinedStep() -> GetProcessName();
		particleName = step -> GetTrack() -> GetDefinition() -> GetParticleName();
		particleType = step -> GetTrack() -> GetDefinition() -> GetParticleType();
		motherId = step -> GetTrack() -> GetParentID();

		stepEnergyDifference =  step -> GetTotalEnergyDeposit() - step->GetNonIonizingEnergyDeposit();//CLHEP::keV;      
		stepTotalEnergy = step -> GetTrack() -> GetTotalEnergy(); //CLHEP::keV  
		stepKineticEnergy = step -> GetTrack() -> GetKineticEnergy(); //CLHEP::keV

		stepNumber = track->GetCurrentStepNumber();

		

		particleId = step -> GetTrack() -> GetTrackID();
		positionPost = step -> GetPostStepPoint() -> GetPosition(); 
		G4String stepProcess = step -> GetPostStepPoint() -> GetProcessDefinedStep() -> GetProcessName();

		// Get the event ID.
		//G4int eventId = -1;
		G4Event const* event = G4RunManager::GetRunManager()->GetCurrentEvent();
		if (event != NULL) {
			eventId = event->GetEventID();
		}
		*m_enteredScoringVolume =
			*m_enteredScoringVolume || (preLogicalVolume == scoringVolume);

		//----------------------------------------------------//
		//----------------------------------------------------//
		//----------------------------------------------------//
		//----------------------------------------------------//
		

		

		if  (stepNumber == 1 ) 
		{
			positionInitial = positionPre;
			initialEkin = step -> GetPreStepPoint() -> GetKineticEnergy();
			if (particleId != 1) {processCreator = step->GetTrack()->GetCreatorProcess()->GetProcessName();}

		
			if (particleId == 1 )
			{
				//cout	<< " 	R = " << sa_Position.getR() << endl;
			
				motherName = "first";
				sa_VecPartName.clear();	
				sa_VecPartName.push_back(particle->GetParticleName());
				sa_VecPartId.clear();
				sa_VecPartId.push_back(particleId);	
				positionRef = positionPre;
				subEventNb = 1;
				refPartName = particle->GetParticleName();
				timeParticle = particle->GetParticleName();
				positionTime = positionPre;
			}
			else
			{
				sa_VecPartName.push_back(particle->GetParticleName());
				sa_VecPartId.push_back(particleId);
				for (G4int k = 0; k <= (G4int) sa_VecPartName.size(); k++)
					{
					if(motherId == sa_VecPartId[k]) {motherName = sa_VecPartName[k];}
					}
			}
		} 

		
		
		//----------------------------------------------------//
		//----------------------------------------------------//
		
		if ((step -> GetPostStepPoint() -> GetLocalTime() - step -> GetPreStepPoint() -> GetLocalTime()) > 1.*CLHEP::millisecond) 
		{
			subEventNb++;
			timeParticle = particle->GetParticleName();
			positionTime = positionPre;
		}
	
		if ((preLogicalVolume == scoringVolume)&&(process != "Transportation"))
    	{

		//----------------------------------------------------//
		//----------------------------------------------------//
		//----------------------------------------------------//
		//----------------------------------------------------//

		// Write the information into the accumulables.
		ParticleScore particleScore {
			
			eventId, //int
			globalTime, // double
			particleName, //string
			particleType, //string 
			initialEkin,  //double
			processCreator, //string
			particleId,   //int
			motherName,	  //string
			motherId,     //int
			positionPre,	// Point3D (decompose to xyz)
			positionPost,  // Point3D (decompose to xyz)
			stepEnergyDifference, //double
			stepTotalEnergy,  //double
			stepKineticEnergy, //double 
			process,	//string
			positionInitial,  //point3D (decompose to xyz)
			refPartName,  //string
			positionRef, //Point3D (decompose to xyz)
			timeParticle, //string
			positionTime,  //Point3D (decompose to xyz)
			subEventNb,   //int

			/*
			eventId,
			track->GetParentID(),
			track->GetTrackID(),
			preLogicalVolume->GetName(),
			preProcessCreator != NULL ? preProcessCreator->GetProcessName() : "NULL",
			particle->GetParticleName(),
			stepNumber,
			time,
			positionPre,
			direction,
			energyDeposit,
			energy,
			*/

			//VARIABLES FROM PARTICLE SCORE HERE

			//stepNumber
		};
		
		
		m_eventParticleScores->Add(particleScore);
		}
	}
}

