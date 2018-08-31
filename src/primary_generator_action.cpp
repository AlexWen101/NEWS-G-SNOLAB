#include "primary_generator_action.h"

#include <cmath>
#include <vector>

#include <G4RunManager.hh>
#include <G4UImanager.hh>
#include <G4ParticleTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4ThreeVector.hh>
#include <G4SystemOfUnits.hh>

#include <CLHEP/Random/Randomize.h>

using namespace shield;



PrimaryGeneratorAction::PrimaryGeneratorAction() {
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
	DestroyDecays();
}

void PrimaryGeneratorAction::CreateDecays() {
	DestroyDecays();
	
	// Get the position from the geometry.
	m_detectorConstruction =
		reinterpret_cast<DetectorConstruction const*>(
			G4RunManager::GetRunManager()->GetUserDetectorConstruction());
	
	// Get the decays from the UI manager.
	G4UImanager* uiManager = G4UImanager::GetMasterUIpointer();
	m_runType = RunTypeFromString(uiManager->SolveAlias("{runType}"));
	m_decays = RunTypeGetDecays(m_runType);
	
	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	for (unsigned int i = 0; i < m_decays.size(); ++i) {
		Decay decay = m_decays[i];
		G4ParticleDefinition* particle =
			particleTable->FindParticle(decay.particle);
		G4ParticleGun* gun = new G4ParticleGun();
		gun->SetParticleDefinition(particle);
		m_guns.push_back(gun);
	}
}

void PrimaryGeneratorAction::DestroyDecays() {
	for (unsigned int i = 0; i < m_guns.size(); ++i) {
		delete m_guns[i];
	}
	m_guns.clear();
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
	
	CLHEP::HepRandomEngine* engine = CLHEP::HepRandom::getTheEngine();
	
	for (unsigned int i = 0; i < m_decays.size(); ++i) {
		Decay& decay = m_decays[i];
		G4ParticleGun* gun = m_guns[i];
		
		if (decay.frequency < engine->flat()) {
			continue;
		}
		
		for (G4int j = 0; j < decay.numParticles; ++j) {
			G4Point3D position;
			G4Vector3D direction;

			G4double energy = decay.spectrum.Draw(engine->flat()); // <-- ALTER PARTICLE ENERGY HERE

			if (!RunTypeIsBackground(m_runType)) {
				// Generate a point at the source position.
				G4double mag2;
				do {
					direction[0] = 2.0 * (engine->flat() - 0.5);
					direction[1] = 2.0 * (engine->flat() - 0.5);
					direction[2] = 2.0 * (engine->flat() - 0.5);
					mag2 = direction.mag2();
				} while (mag2 > 1.0 || mag2 < 0.001);
				
				position = m_detectorConstruction->GetSourcePosition();
			}
			else {
				// Generate a random point on an n-sided regular prism.
				ShieldingInfo shieldingInfo =
					m_detectorConstruction->GetShieldingInfo();
				
				G4int numSides = shieldingInfo.hdpeShieldNumSides;
				G4double height =
					shieldingInfo.hdpeShieldInnerHeight +
					2 * shieldingInfo.hdpeShieldThickness;
				G4double radius =
					shieldingInfo.hdpeShieldInnerRadius +
					shieldingInfo.hdpeShieldThickness;
				
				// Angle of one isosceles triangle making up the face of the
				// prism.
				G4double angle = 360.0*deg / numSides;
				// Length of one edge of the face of the prism.
				G4double edgeLength = 2.0 * radius * std::tan(angle / 2.0);
				
				// Choose whether generating on the top/bottom, or on the side.
				G4double faceArea = 2.0 * (0.5 * radius * edgeLength);
				G4double sideArea = edgeLength * height;
				if (engine->flat() < faceArea / (faceArea + sideArea)) {
					// It's going on the face.
					position[0] = edgeLength * (engine->flat() - 0.5);
					position[1] = radius * engine->flat();
					G4double ratio = (position[1] == 0.0) ?
						0.0 :
						std::abs(position[0] / position[1]);
					G4double ratioSign = (position[0] > 0.0) ? +1.0 : -1.0;
					if (ratio > 0.5 * edgeLength / radius) {
						position[0] = 0.5 * ratioSign * edgeLength - position[0];
						position[1] = radius - position[1];
					}
					// Choose top or bottom face randomly.
					G4double heightSign = (engine->flat() > 0.5) ? +1.0 : -1.0;
					position[2] = heightSign * (0.5 * height + 0.1*mm);
					
					// Direction facing in the half-sphere towards detector.
					G4double mag2;
					do {
						direction[0] = 2.0 * (engine->flat() - 0.5);
						direction[1] = 2.0 * (engine->flat() - 0.5);
						direction[2] = -heightSign * engine->flat();
						mag2 = direction.mag2();
					} while (mag2 > 1.0 || mag2 < 0.001);
				}
				else {
					// It's going on the side.
					position[0] = edgeLength * (engine->flat() - 0.5);
					position[1] = radius + 0.1*mm;
					position[2] = height * (engine->flat() - 0.5);
					
					// Direction facing in the half-sphere towards detector.
					G4double mag2;
					do {
						direction[0] = 2.0 * (engine->flat() - 0.5);
						direction[1] = -engine->flat();
						direction[2] = 2.0 * (engine->flat() - 0.5);
						mag2 = direction.mag2();
					} while (mag2 > 1.0 || mag2 < 0.001);
				}
				
				// Randomly choose which side of the prism.
				G4int side = (G4int) (numSides * engine->flat());
				if (side < 0) {
					side = 0;
				}
				else if (side >= numSides) {
					side = numSides - 1;
				}
				
				// Rotate the point and direction based on the side.
				G4Transform3D transform = G4RotateZ3D(side * angle);
				position.transform(transform);
				direction.transform(transform);
				
				/*
				position = G4Point3D(
					2.0 * (engine->flat() - 0.5),
					2.0 * (engine->flat() - 0.5),
					2.0 * (engine->flat() - 0.5));
				G4int side = (G4int) (3.0 * engine->flat());
				if (side < 0) side = 0;
				if (side > 2) side = 2;
				G4double sideSign = (engine->flat() > 0.5) ? +1.0 : -1.0;
				position[side] = sideSign;
				position *=
					0.5 * (m_detectorConstruction->GetWorldSize() - 0.1*mm);
				
				G4double mag2;
				do {
					for (G4int dim = 0; dim < 3; ++dim) {
						if (dim != side) {
							direction[dim] = 2.0 * (engine->flat() - 0.5);
						}
						else {
							direction[dim] = -sideSign * engine->flat();
						}
					}
					mag2 = direction.mag2();
				} while (mag2 > 1.0 || mag2 < 0.001);
				*/
			}
			gun->SetParticlePosition(position);
			gun->SetParticleMomentumDirection(direction);
			gun->SetParticleEnergy(energy);
			gun->GeneratePrimaryVertex(event);
		}
	}
}

