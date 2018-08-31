#ifndef __SHIELD_DETECTOR_CONSTRUCTION_H_
#define __SHIELD_DETECTOR_CONSTRUCTION_H_

#include <G4VUserDetectorConstruction.hh>
#include <G4LogicalVolume.hh>
#include <G4Point3D.hh>
#include <G4Types.hh>
#include <G4SystemOfUnits.hh>

namespace shield {

struct ShieldingInfo {
	G4double detectorRadius        = 675*mm;
	G4double copperSphereThickness = 15*mm;
	G4double leadShieldThickness   = 272*mm;
	G4double hdpeShieldInnerRadius = 1225*mm;
	G4double hdpeShieldInnerHeight = 1990*mm;
	G4double hdpeShieldThickness   = 400*mm;
	G4int hdpeShieldNumSides       = 8;
};

struct GloveboxInfo {
	// Inner size.
	G4double size                 = 9.5*25.4*mm;
	G4double steelShieldThickness = 2*mm;
	G4double hdpeShieldThickness  = 50*mm;
	G4double leadShieldThickness  = 3*mm;
};

struct TubeInfo {
	// Inner radius.                        defaults
	bool tubeExists          = true;        // true
	G4double radius          = 13.5*mm;     // 13.5*mm
	G4double thickness       = 2.5*mm;      // 2.5*mm
	G4double headLength      = 150*mm;      // 150*mm
	G4double headBendRadius  = 600*mm;      // 600*mm
	G4double middleLength    = 763.3*mm;    // 763.3*mm
	G4double tailbendRadius  = 400*mm;      // 400*mm
	G4double tailLength      = 300*mm;      // 300*mm
	G4double bendAngle       = 55*deg;      // 55*deg
	G4double verticalAngle   = 50*deg;      // 50*deg
	G4double sphereSpacing   = 8.3*mm;      // 8.3*mm
	G4double gloveboxSpacing = 10*mm;       // 10*mm
};

class DetectorConstruction : public G4VUserDetectorConstruction {
	
public:
	
	DetectorConstruction(
		G4double worldSize,
		ShieldingInfo shieldingInfo,
		GloveboxInfo gloveboxInfo,
		TubeInfo tubeInfo) :
		m_worldSize(worldSize),
		m_shieldingInfo(shieldingInfo),
		m_gloveboxInfo(gloveboxInfo),
		m_tubeInfo(tubeInfo) {
	}
	
	virtual G4VPhysicalVolume* Construct();
	
	G4LogicalVolume* GetScoringVolume() {
		return m_scoringVolume;
	}
	
	G4Point3D GetSourcePosition() const {
		return m_sourcePosition;
	}
	
	G4double GetWorldSize() const {
		return m_worldSize;
	}
	
	ShieldingInfo GetShieldingInfo() const {
		return m_shieldingInfo;
	}
	GloveboxInfo GetGloveboxInfo() const {
		return m_gloveboxInfo;
	}
	TubeInfo GetTubeInfo() const {
		return m_tubeInfo;
	}
	
private:
	
	G4double m_worldSize;
	ShieldingInfo m_shieldingInfo;
	GloveboxInfo m_gloveboxInfo;
	TubeInfo m_tubeInfo;
	G4Point3D m_sourcePosition;
	G4LogicalVolume* m_scoringVolume;
};

}

#endif

