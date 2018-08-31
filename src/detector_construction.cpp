#include "detector_construction.h"

#include <G4UImanager.hh>

#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VSolid.hh>
#include <G4Sphere.hh>
#include <G4Orb.hh>
#include <G4Box.hh>
#include <G4Polyhedra.hh>
#include <G4Torus.hh>
#include <G4Tubs.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4NistManager.hh>
#include <G4Material.hh>
#include <G4Transform3D.hh>
#include <G4Types.hh>
#include <G4Point3D.hh>
#include <G4Vector3D.hh>
#include <G4SystemOfUnits.hh>

#include <cmath>

using namespace shield;

G4VPhysicalVolume* DetectorConstruction::Construct() {
	
	G4NistManager* nist = G4NistManager::Instance();
	
	// Get the elements for making our materials.
	G4Element* elementFe = nist->FindOrBuildElement("Fe");
	G4Element* elementCr = nist->FindOrBuildElement("Cr");
	G4Element* elementNi = nist->FindOrBuildElement("Ni");
	G4Element* elementMn = nist->FindOrBuildElement("Mn");
	G4Element* elementSi = nist->FindOrBuildElement("Si");
	
	G4Element* elementH  = nist->FindOrBuildElement("H");
	G4Element* elementC  = nist->FindOrBuildElement("C");
	G4Element* elementB  = nist->FindOrBuildElement("B");
	
	// Create the materials.
	G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
	G4Material* copper = nist->FindOrBuildMaterial("G4_Cu");
	G4Material* lead  = nist->FindOrBuildMaterial("G4_Pb");
	G4Material* neon = nist->ConstructNewGasMaterial(
		"G4_Ne", "G4_Ne",
		NTP_Temperature,
		2*bar);
	G4Material* stainlessSteel = new G4Material(
		"StainlessSteel",
		8.03*g/cm3,
		5,
		G4State::kStateSolid);
	stainlessSteel->AddElement(elementFe, 68.75*perCent);
	stainlessSteel->AddElement(elementCr, 19.00*perCent);
	stainlessSteel->AddElement(elementNi,  9.25*perCent);
	stainlessSteel->AddElement(elementMn,  2.00*perCent);
	stainlessSteel->AddElement(elementSi,  1.00*perCent);
	
	G4Material* hdpe = new G4Material(
		"HDPE",
		0.95*g/cm3,
		2,
		G4State::kStateSolid);
	hdpe->AddElement(elementC, 2);
	hdpe->AddElement(elementH, 4);
	
	G4Material* hdpeBoron = new G4Material(
		"HDPEBoron",
		0.95*g/cm3,
		2,
		G4State::kStateSolid);
	hdpeBoron->AddMaterial(hdpe, 95*perCent);
	hdpeBoron->AddElement(elementB, 5*perCent);
	
	// Define the parameters of the geometry.
	
	// For the spherical detector + shielding.
	G4double worldSize = m_worldSize / 2.0;
	G4double detectorRadius = m_shieldingInfo.detectorRadius;
	G4double copperSphereRadius =
		detectorRadius + m_shieldingInfo.copperSphereThickness;
	G4double leadShieldRadius =
		copperSphereRadius + m_shieldingInfo.leadShieldThickness;
	G4double hdpeShieldInnerRadius = m_shieldingInfo.hdpeShieldInnerRadius;
	G4double hdpeShieldOuterRadius =
		hdpeShieldInnerRadius + m_shieldingInfo.hdpeShieldThickness;
	G4int hdpeShieldNumSides = m_shieldingInfo.hdpeShieldNumSides;
	G4double hdpeShieldInnerZ = m_shieldingInfo.hdpeShieldInnerHeight / 2.0;
	G4double hdpeShieldOuterZ =
		hdpeShieldInnerZ + m_shieldingInfo.hdpeShieldThickness;
	
	// For the glovebox.
	G4double boxSize = m_gloveboxInfo.size / 2.0;
	G4double boxSteelShieldOuter =
		boxSize + m_gloveboxInfo.steelShieldThickness;
	G4double boxHdpeShieldOuter =
		boxSteelShieldOuter + m_gloveboxInfo.hdpeShieldThickness;
	G4double boxLeadShieldOuter =
		boxHdpeShieldOuter + m_gloveboxInfo.leadShieldThickness;;
	
	// For the copper tube.
	bool tubeExists = m_tubeInfo.tubeExists;
	G4double tubeInnerRadius = m_tubeInfo.radius;
	G4double tubeOuterRadius = tubeInnerRadius + m_tubeInfo.thickness;
	G4double tubeHeadLength = m_tubeInfo.headLength;
	G4double tubeHeadBendRadius = m_tubeInfo.headBendRadius;
	G4double tubeMiddleLength = m_tubeInfo.middleLength;
	G4double tubeTailBendRadius = m_tubeInfo.tailbendRadius;
	G4double tubeTailLength = m_tubeInfo.tailLength;
	G4double tubeBendAngle = m_tubeInfo.bendAngle;
	G4double tubeVerticalAngle = m_tubeInfo.verticalAngle;
	G4double tubeSphereSpacing = m_tubeInfo.sphereSpacing;
	G4double tubeBoxSpacing = m_tubeInfo.gloveboxSpacing;
	
	/*=== TRANSFORMATIONS ===*/
	
	// Prepare the transformations for parts of the tube. The tube goes in the
	// +z direction and lies in the x-z plane.
	G4Transform3D tubeHeadModelTransform = G4Transform3D();
	G4Transform3D tubeHeadBendModelTransform =
		G4RotateZ3D(180*deg) *
		G4RotateX3D(+90*deg) *
		G4TranslateX3D(-tubeHeadBendRadius);
	G4Transform3D tubeMiddleModelTransform =
		G4TranslateZ3D(+tubeMiddleLength / 2.0);
	G4Transform3D tubeTailBendModelTransform =
		G4RotateX3D(+90*deg) *
		G4TranslateX3D(-tubeTailBendRadius);
	G4Transform3D tubeTailModelTransform =
		G4TranslateZ3D(+tubeTailLength / 2.0);
	
	// The local transforms are used to position the tube parts relative to each
	// other. The tube plane is the x-z plane, with +z being forward.
	G4Transform3D tubeHeadLocalTransform = G4Transform3D();
	G4Transform3D tubeHeadBendLocalTransform =
		tubeHeadLocalTransform *
		G4TranslateZ3D(+tubeHeadLength / 2.0);
	G4Transform3D tubeMiddleLocalTransform =
		tubeHeadBendLocalTransform *
		G4TranslateX3D(+tubeHeadBendRadius) *
		G4RotateY3D(+tubeBendAngle) *
		G4TranslateX3D(-tubeHeadBendRadius);
	G4Transform3D tubeTailBendLocalTransform =
		tubeMiddleLocalTransform *
		G4TranslateZ3D(+tubeMiddleLength);
	G4Transform3D tubeTailLocalTransform =
		tubeTailBendLocalTransform *
		G4TranslateX3D(-tubeTailBendRadius) *
		G4RotateY3D(-tubeBendAngle) *
		G4TranslateX3D(+tubeTailBendRadius);
	G4Transform3D tubeEndLocalTransform =
		tubeTailLocalTransform *
		G4TranslateZ3D(+tubeTailLength);
	
	// The world transform positions the tube as a whole.
	G4Transform3D tubeWorldTransform =
		G4TranslateY3D(tubeOuterRadius + tubeSphereSpacing) *
		G4TranslateY3D(copperSphereRadius) *
		G4RotateX3D(+270*deg - tubeVerticalAngle) *
		G4TranslateX3D(+tubeHeadLength / 2.0) *
		G4RotateY3D(+90*deg);


// Box transform is defined here as default, but will be updated later to fit the location. 
G4Transform3D boxTransform =
			G4TranslateX3D(-tubeBoxSpacing) *
			G4TranslateX3D(+boxSize) *
			G4Translate3D(
				(tubeWorldTransform * tubeEndLocalTransform).getTranslation());

//--------------// //--------------// //--------------// //--------------// //--------------//
//--------------// //--------------// //--------------// //--------------// //--------------//
// CHANGING THE SOURCE LOCATION 
//--------------// //--------------// //--------------// //--------------// //--------------//
//--------------// //--------------// //--------------// //--------------// //--------------//

	G4UImanager* uiManager = G4UImanager::GetMasterUIpointer();

	G4String runLocation = uiManager->SolveAlias("{runLocation}"); // The location value is read here from the configure_commands macro 

	//--------------// //--------------// //--------------// //--------------// //--------------//
	// LOCATION 1: INSIDE HDPE, FAR FROM TUBE
	//--------------// //--------------// //--------------// //--------------// //--------------//

	if (runLocation == "origin") {

		boxTransform =
			G4TranslateX3D(-tubeBoxSpacing) *
			G4TranslateX3D(+boxSize) *
			G4Translate3D(
				(tubeWorldTransform * tubeEndLocalTransform).getTranslation());

		m_sourcePosition = G4Translate3D(0*cm,0*cm,0*cm) * G4Point3D();

	}

	else if (runLocation == "location1") { 
	
		boxTransform =
			G4TranslateX3D(-tubeBoxSpacing) *
			G4TranslateX3D(+boxSize) *
			G4Translate3D(
				(tubeWorldTransform * tubeEndLocalTransform).getTranslation());

		m_sourcePosition = G4Translate3D(-76.222*cm,0*cm,-68.600*cm) * G4Point3D();
	} 

	//--------------// //--------------// //--------------// //--------------// //--------------//
	// LOCATION 2: INSIDE HDPE, CLOSE TO TUBE
	//--------------// //--------------// //--------------// //--------------// //--------------//

	else if (runLocation == "location2") {
		boxTransform =
			G4TranslateX3D(-tubeBoxSpacing) *
			G4TranslateX3D(+boxSize) *
			G4Translate3D(
				(tubeWorldTransform * tubeEndLocalTransform).getTranslation());

		// These coordinates were decided to ensure that a) the position is inside the tube and b) the distance to the sphere
		// is the same as the distance of location 1 to ensure that the solid angle is constant so the two locations can be compared. 
		m_sourcePosition = G4Translate3D(86.037*cm,34.890*cm,43.545*cm) * G4Point3D();  
	} 

	//--------------// //--------------// //--------------// //--------------// //--------------//
	// LOCATION 3: OUTSIDE HDPE, FAR FROM TUBE
	//--------------// //--------------// //--------------// //--------------// //--------------//

	else if (runLocation == "location3") {
		G4Transform3D tubeHeadLocalTransformO = G4Transform3D();
		G4Transform3D tubeHeadBendLocalTransformO =
			tubeHeadLocalTransformO *
			G4TranslateZ3D(+tubeHeadLength / 2.0);
		G4Transform3D tubeMiddleLocalTransformO =
			tubeHeadBendLocalTransformO *
			G4TranslateX3D(-tubeHeadBendRadius) *
			G4RotateY3D(-tubeBendAngle) *
			G4TranslateX3D(+tubeHeadBendRadius);
		G4Transform3D tubeTailBendLocalTransformO =
			tubeMiddleLocalTransformO *
			G4TranslateZ3D(+tubeMiddleLength);
		G4Transform3D tubeTailLocalTransformO =
			tubeTailBendLocalTransformO *
			G4TranslateX3D(+tubeTailBendRadius) *
			G4RotateY3D(+tubeBendAngle) *
			G4TranslateX3D(-tubeTailBendRadius);
		G4Transform3D tubeEndLocalTransformO =
			tubeTailLocalTransformO *
			G4TranslateZ3D(+tubeTailLength);

		G4Transform3D tubeWorldTransformO = 
			G4TranslateY3D(tubeOuterRadius + tubeSphereSpacing) *
			G4TranslateY3D(copperSphereRadius) *
			G4RotateX3D(+270*deg - tubeVerticalAngle) *
			G4TranslateX3D(-tubeHeadLength / 2.0) *
			G4RotateY3D(-90*deg);

		// Transformation for the glovebox on the end of the tube. 
		boxTransform =
			G4TranslateX3D(-tubeBoxSpacing) *
		// GLOVEBOX TRANSLATION (drag that shit out of the way!!!) 
			G4TranslateX3D(+boxSize) *
			G4Translate3D(
				(tubeWorldTransform * tubeEndLocalTransform).getTranslation());
		// Translated source exactly symmetric to glovebox source in y-z plane
		m_sourcePosition = 
			G4TranslateX3D(+tubeBoxSpacing) * 
			G4TranslateX3D(-boxSize) *
			G4Translate3D(
				(tubeWorldTransformO * tubeEndLocalTransformO).getTranslation()) *
			G4Point3D(); 

		std::cout << "source position is: " << m_sourcePosition << std::endl; 
	}

	//--------------// //--------------// //--------------// //--------------// //--------------//
	// LOCATION 4: OUTSIDE HDPE, CLOSE TO TUBE
	//--------------// //--------------// //--------------// //--------------// //--------------//

	else if (runLocation == "location4") { 
		boxTransform =
			G4TranslateX3D(-tubeBoxSpacing) *
			G4TranslateX3D(+boxSize) *
			G4TranslateX3D(+0.5*m) *
			G4Translate3D(
				(tubeWorldTransform * tubeEndLocalTransform).getTranslation());
		// Translated source exactly symmetric to glovebox source in y-z plane
		m_sourcePosition = boxTransform * G4TranslateX3D(-0.5*m) * G4Point3D();

		std::cout << "source position is: " << m_sourcePosition << std::endl; 
	} 

	

	//--------------// //--------------// //--------------// //--------------// //--------------//
	// LOCATION 5: IN DEPLOYED POSITION: JUST OUTSIDE COPPER SPHERE, AT THE END (HEAD) OF THE COPPER TUBE
	//--------------// //--------------// //--------------// //--------------// //--------------//

	else if (runLocation == "location5") { 
		boxTransform =
			G4TranslateX3D(-tubeBoxSpacing) *
			G4TranslateX3D(+boxSize) *
			G4Translate3D(
				(tubeWorldTransform * tubeEndLocalTransform).getTranslation());
		
		
		// Get the translation of the Tube Head and set the source position equal to that position. 

		G4Translate3D translation = G4Translate3D(); 
		G4Rotate3D rotation = G4Rotate3D(); 
		G4Scale3D scale = G4Scale3D(); 

		(tubeWorldTransform * tubeHeadLocalTransform).getDecomposition(scale,rotation,translation);//default one

		//(tubeWorldTransform * tubeMiddleLocalTransform).getDecomposition(scale,rotation,translation); //delete this when done with the gammas

		m_sourcePosition = translation * G4TranslateX3D(-55*mm) * G4Point3D();//this is the default one

		//m_sourcePosition = G4Translate3D(0*mm,-820*mm,0*mm)*G4Point3D();//translation * G4Point3D();//delete this when done with the gammas

		std::cout << "source position is: " << m_sourcePosition << std::endl; 
	} 

	//--------------// //--------------// //--------------// //--------------// //--------------//
	// LOCATION 6: INSIDE HDPE, DIRECTLY UNDERNEATH LEAD SHIELD
	//--------------// //--------------// //--------------// //--------------// //--------------//

	else if (runLocation == "location6") {
		boxTransform =
			G4TranslateX3D(-tubeBoxSpacing) *
			G4TranslateX3D(+boxSize) *
			G4Translate3D(
				(tubeWorldTransform * tubeEndLocalTransform).getTranslation());

		m_sourcePosition = G4Translate3D(0*cm,0*cm,-978.5*mm) * G4Point3D();  
	}

	//--------------// //--------------// //--------------// //--------------// //--------------//
	//--------------// //--------------// //--------------// //--------------// //--------------//

	else { 
		// DEFAULT position (same as location 4 but with glovebox surrounding it)
	
		boxTransform =
			G4TranslateX3D(-tubeBoxSpacing) *
			G4TranslateX3D(+boxSize) *
			G4Translate3D(
				(tubeWorldTransform * tubeEndLocalTransform).getTranslation());
		m_sourcePosition = boxTransform * G4Point3D();
	}
	//--------------// //--------------// //--------------// //--------------// //--------------//
	//--------------// //--------------// //--------------// //--------------// //--------------//
	//--------------// //--------------// //--------------// //--------------// //--------------//
	//--------------// //--------------// //--------------// //--------------// //--------------//
		


	// Create the world.
	G4VSolid* worldShape = new G4Box(
		"World", worldSize, worldSize, worldSize);
	G4LogicalVolume* worldLogical = new G4LogicalVolume(
		worldShape, air, "World");
	G4VPhysicalVolume* worldPhysical = new G4PVPlacement(
		0,
		G4Vector3D(),
		worldLogical,
		"World",
		NULL,
		false,
		0);
	
	/*=== COPPER TUBE ===*/
	
	G4VSolid* tubeHeadShape = new G4Tubs(
		"TubeHead",
		tubeInnerRadius, tubeOuterRadius,
		tubeHeadLength / 2.0,
		0.0, 360*deg);
	G4VSolid* tubeHeadBendShape = new G4Torus(
		"TubeHeadBend",
		tubeInnerRadius, tubeOuterRadius,
		tubeHeadBendRadius,
		0.0, tubeBendAngle);
	G4VSolid* tubeMiddleShape = new G4Tubs(
		"TubeMiddle",
		tubeInnerRadius, tubeOuterRadius,
		tubeMiddleLength / 2.0,
		0.0, 360*deg);
	G4VSolid* tubeTailBendShape = new G4Torus(
		"TubeTailBend",
		tubeInnerRadius, tubeOuterRadius,
		tubeTailBendRadius,
		0.0, tubeBendAngle);
	G4VSolid* tubeTailShape = new G4Tubs(
		"TubeTail",
		tubeInnerRadius, tubeOuterRadius,
		tubeTailLength / 2.0,
		0.0, 360*deg);
	
	G4VSolid* tubeShape =
		new G4UnionSolid(
			"Tube",
			new G4UnionSolid(
				"TubeHeadBendMiddleBendUnion",
				new G4UnionSolid(
					"TubeHeadBendMiddleUnion",
					new G4UnionSolid(
						"TubeHeadBendUnion",
						tubeHeadShape,
						tubeHeadBendShape,
						tubeHeadBendLocalTransform * tubeHeadBendModelTransform),
					tubeMiddleShape,
					tubeMiddleLocalTransform * tubeMiddleModelTransform),
				tubeTailBendShape,
				tubeTailBendLocalTransform * tubeTailBendModelTransform),
			tubeTailShape,
			tubeTailLocalTransform * tubeTailModelTransform);
	
	// The solid tube is used to hollow out space for the real tube to go in the
	// lead shield and the HDPE shield.
	G4Tubs* solidTubeHeadShape =
		reinterpret_cast<G4Tubs*>(tubeHeadShape->Clone());
	G4Tubs* solidTubeMiddleShape =
		reinterpret_cast<G4Tubs*>(tubeMiddleShape->Clone());
	G4Tubs* solidTubeTailShape =
		reinterpret_cast<G4Tubs*>(tubeTailShape->Clone());
	G4Torus* solidTubeHeadBendShape =
		reinterpret_cast<G4Torus*>(tubeHeadBendShape->Clone());
	G4Torus* solidTubeTailBendShape =
		reinterpret_cast<G4Torus*>(tubeTailBendShape->Clone());
	
	solidTubeHeadShape->SetInnerRadius(0.0);
	solidTubeMiddleShape->SetInnerRadius(0.0);
	solidTubeTailShape->SetInnerRadius(0.0);
	solidTubeHeadBendShape->SetAllParameters(
		0.0,
		solidTubeHeadBendShape->GetRmax(),
		solidTubeHeadBendShape->GetRtor(),
		solidTubeHeadBendShape->GetSPhi(),
		solidTubeHeadBendShape->GetDPhi());
	solidTubeTailBendShape->SetAllParameters(
		0.0,
		solidTubeTailBendShape->GetRmax(),
		solidTubeTailBendShape->GetRtor(),
		solidTubeTailBendShape->GetSPhi(),
		solidTubeTailBendShape->GetDPhi());
	
	solidTubeHeadShape->SetName("SolidTubeHead");
	solidTubeMiddleShape->SetName("SolidTubeMiddle");
	solidTubeTailShape->SetName("SolidTubeTail");
	solidTubeHeadBendShape->SetName("SolidTubeHeadBend");
	solidTubeTailBendShape->SetName("SolidTubeTailBend");
	
	G4VSolid* solidTubeShape =
		new G4UnionSolid(
			"SolidTube",
			new G4UnionSolid(
				"SolidTubeHeadBendMiddleBendUnion",
				new G4UnionSolid(
					"SolidTubeHeadBendMiddleUnion",
					new G4UnionSolid(
						"SolidTubeHeadBendUnion",
						solidTubeHeadShape,
						solidTubeHeadBendShape,
						tubeHeadBendLocalTransform * tubeHeadBendModelTransform),
					solidTubeMiddleShape,
					tubeMiddleLocalTransform * tubeMiddleModelTransform),
				solidTubeTailBendShape,
				tubeTailBendLocalTransform * tubeTailBendModelTransform),
			solidTubeTailShape,
			tubeTailLocalTransform * tubeTailModelTransform);
	
	G4LogicalVolume* tubeLogical = NULL;
	G4VPhysicalVolume* tubePhysical = NULL;
	if (tubeExists) {
		tubeLogical = new G4LogicalVolume(
			tubeShape, copper, "Tube");
		tubePhysical = new G4PVPlacement(
			tubeWorldTransform,
			tubeLogical,
			"Tube",
			worldLogical,
			false,
			0);
	}
	
	/*=== DETECTOR AND SHIELDING ===*/
	
	// Create an orb for the lead shield.
	G4VSolid* leadShieldShape;
	if (tubeExists) {
			leadShieldShape =  new G4SubtractionSolid(
				"LeadShield",
				new G4Orb("LeadShieldIntact", leadShieldRadius),
				solidTubeShape,
				tubeWorldTransform);
	}
	else {
		leadShieldShape = new G4Orb("LeadShield", leadShieldRadius);
	}
	G4LogicalVolume* leadShieldLogical = new G4LogicalVolume(
		leadShieldShape, lead, "LeadShield");
	G4VPhysicalVolume* leadShieldPhysical = new G4PVPlacement(
		0,
		G4Vector3D(),
		leadShieldLogical,
		"LeadShield",
		worldLogical,
		false,
		0);
	
	// Create an orb for the copper sphere.
	G4VSolid* copperSphereShape = new G4Orb(
		"CopperSphere", copperSphereRadius);
	G4LogicalVolume* copperSphereLogical = new G4LogicalVolume(
		copperSphereShape, copper, "CopperSphere");
	G4VPhysicalVolume* copperSpherePhysical = new G4PVPlacement(
		0,
		G4Vector3D(),
		copperSphereLogical,
		"CopperSphere",
		leadShieldLogical,
		false,
		0);
	
	// Create an orb for the detector.
	G4VSolid* detectorShape = new G4Orb(
		"Detector", detectorRadius);
	G4LogicalVolume* detectorLogical = new G4LogicalVolume(
		detectorShape, neon, "Detector");
	G4VPhysicalVolume* detectorPhysical = new G4PVPlacement(
		0,
		G4Vector3D(),
		detectorLogical,
		"Detector",
		copperSphereLogical,
		false,
		0);
	
	// Create the HDPE shield.
	G4double zPlane[] = {
		-hdpeShieldOuterZ, -hdpeShieldInnerZ, -hdpeShieldInnerZ,
		+hdpeShieldInnerZ, +hdpeShieldInnerZ, +hdpeShieldOuterZ
	};
	G4double rInner[] = {
		0.0, 0.0, hdpeShieldInnerRadius,
		hdpeShieldInnerRadius, 0.0, 0.0
	};
	G4double rOuter[] = {
		hdpeShieldOuterRadius, hdpeShieldOuterRadius, hdpeShieldOuterRadius,
		hdpeShieldOuterRadius, hdpeShieldOuterRadius, hdpeShieldOuterRadius
	};
	G4VSolid* hdpeShieldShape;
	if (tubeExists) {
		hdpeShieldShape = new G4SubtractionSolid(
			"HDPEShield",
			new G4Polyhedra(
				"HDPEShieldIntact",
				360*deg/(2 * hdpeShieldNumSides), 360*deg,
				hdpeShieldNumSides,
				6, zPlane, rInner, rOuter),
			solidTubeShape,
			tubeWorldTransform);
	}
	else {
		hdpeShieldShape = new G4Polyhedra(
			"HDPEShield",
			360*deg/(2 * hdpeShieldNumSides), 360*deg,
			hdpeShieldNumSides,
			6, zPlane, rInner, rOuter);
	}
	G4LogicalVolume* hdpeShieldLogical = new G4LogicalVolume(
		hdpeShieldShape, hdpe, "HDPEShield");
	G4VPhysicalVolume* hdpeShieldPhysical = new G4PVPlacement(
		0,
		G4Vector3D(),
		hdpeShieldLogical,
		"HDPEShield",
		worldLogical,
		false,
		0);
	
	/*=== GLOVEBOX ===*/
	
	// Create a box for the glovebox lead shielding.
	G4VSolid* boxLeadShieldShape;
	if (tubeExists) {
		boxLeadShieldShape = new G4SubtractionSolid(
			"BoxLeadShield",
			new G4Box(
				"BoxLeadShieldIntact",
				boxLeadShieldOuter, boxLeadShieldOuter, boxLeadShieldOuter),
			solidTubeShape,
			boxTransform.inverse() * tubeWorldTransform);
	}
	else {
		boxLeadShieldShape = new G4Box(
			"BoxLeadShield",
			boxLeadShieldOuter, boxLeadShieldOuter, boxLeadShieldOuter);
	}
	G4LogicalVolume* boxLeadShieldLogical = new G4LogicalVolume(
		boxLeadShieldShape, lead, "BoxLeadShield");
	G4VPhysicalVolume* boxLeadShieldPhysical = new G4PVPlacement(
		boxTransform,
		boxLeadShieldLogical,
		"BoxLeadShield",
		worldLogical,
		false,
		0);
	
	// Create a box for the glovebox HDPE shielding.
	G4VSolid* boxHdpeShieldShape;
	if (tubeExists) {
		boxHdpeShieldShape = new G4SubtractionSolid(
			"BoxHDPEShield",
			new G4Box(
				"BoxHDPEShieldIntact",
				boxHdpeShieldOuter, boxHdpeShieldOuter, boxHdpeShieldOuter),
			solidTubeShape,
			boxTransform.inverse() * tubeWorldTransform);
	}
	else {
		boxHdpeShieldShape = new G4Box(
			"BoxHDPEShield",
			boxHdpeShieldOuter, boxHdpeShieldOuter, boxHdpeShieldOuter);
	}
	G4LogicalVolume* boxHdpeShieldLogical = new G4LogicalVolume(
		boxHdpeShieldShape, hdpeBoron, "BoxHDPEShield");
	G4VPhysicalVolume* boxHdpeShieldPhysical = new G4PVPlacement(
		0,
		G4Vector3D(),
		boxHdpeShieldLogical,
		"BoxHDPEShield",
		boxLeadShieldLogical,
		false,
		0);
	
	// Create a box for the glovebox steel shielding.
	G4VSolid* boxSteelShieldShape;
	if (tubeExists) {
		boxSteelShieldShape = new G4SubtractionSolid(
			"BoxSteelShield",
			new G4Box(
				"BoxSteelShieldIntact",
				boxSteelShieldOuter, boxSteelShieldOuter, boxSteelShieldOuter),
			solidTubeShape,
			boxTransform.inverse() * tubeWorldTransform);
	}
	else {
		boxSteelShieldShape = new G4Box(
			"BoxSteelShield",
			boxSteelShieldOuter, boxSteelShieldOuter, boxSteelShieldOuter);
	}
	G4LogicalVolume* boxSteelShieldLogical = new G4LogicalVolume(
		boxSteelShieldShape, stainlessSteel, "BoxSteelShield");
	G4VPhysicalVolume* boxSteelShieldPhysical = new G4PVPlacement(
		0,
		G4Vector3D(),
		boxSteelShieldLogical,
		"BoxSteelShield",
		boxHdpeShieldLogical,
		false,
		0);
	
	// Create the interior of the glovebox.
	G4VSolid* boxShape;
	if (tubeExists) {
		boxShape = new G4SubtractionSolid(
		"Box",
		new G4Box(
			"BoxIntact",
			boxSize, boxSize, boxSize),
		solidTubeShape,
		boxTransform.inverse() * tubeWorldTransform);
	}
	else {
		boxShape = new G4Box("Box", boxSize, boxSize, boxSize);
	}
	G4LogicalVolume* boxLogical = new G4LogicalVolume(
		boxShape, air, "Box");
	G4VPhysicalVolume* boxPhysical = new G4PVPlacement(
		0,
		G4Vector3D(),
		boxLogical,
		"Box",
		boxSteelShieldLogical,
		false,
		0);
	
	m_scoringVolume = detectorLogical;
	
	return worldPhysical;
}

