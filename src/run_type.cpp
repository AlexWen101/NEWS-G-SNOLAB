#include "run_type.h"

#include <G4SystemOfUnits.hh>

using namespace shield;

G4String shield::RunTypeToString(RunType runType) {
	switch (runType) {
	case RunType::AmBeNeutron:
		return "AmBeNeutron";
	case RunType::DDNeutron:
		return "DDNeutron";
	case RunType::AmBeCaptureGamma:
		return "AmBeCaptureGamma";
	case RunType::AmBeGamma:
		return "AmBeGamma";
	case RunType::Na22Gamma:
		return "Na22Gamma";
	case RunType::BackgroundNeutron:
		return "BackgroundNeutron";
	default:
		return "Unknown";
	}
}

RunType shield::RunTypeFromString(G4String string) {
	string.toLower();
	if (string == "ambeneutron") {
		return RunType::AmBeNeutron;
	}
	else if (string == "ddneutron") {
		return RunType::DDNeutron;
	}
	else if (string == "ambecapturegamma") {
		return RunType::AmBeCaptureGamma;
	}
	else if (string == "ambegamma") {
		return RunType::AmBeGamma;
	}
	else if (string == "na22gamma") {
		return RunType::Na22Gamma;
	}
	else if (string == "backgroundneutron") {
		return RunType::BackgroundNeutron;
	}
	else {
		return RunType::Unknown;
	}
}

G4double shield::RunTypeGetActivity(RunType runType) {
	switch (runType) {
	case RunType::AmBeNeutron:
		return 620*becquerel;
	case RunType::DDNeutron:
		return 620*becquerel;  // NOT SURE ABOUT THIS VALUE, NEED TO UPDATE WITH MORE ACCURATE ONE
	case RunType::AmBeCaptureGamma:
		// Gamma is emitted from alpha capture 57.5% of the time.
		return 620*becquerel * 57.5*perCent;
	case RunType::AmBeGamma:
		// Gamma is only emitted 75.44% of the time from Am-241 (from
		// integrating the Am-241 gamma spectrum).
		return 11.1e6*becquerel * 75.44*perCent;
	case RunType::Na22Gamma:
		return 37.0e3*becquerel;
	case RunType::BackgroundNeutron:
		return 4000.0/(60.0*60.0*24.0)*becquerel;
	default:
		return 0.0;
	}
}

std::vector<Decay> shield::RunTypeGetDecays(RunType runType) {
	std::vector<Decay> decays;
	switch (runType) {
	case RunType::Na22Gamma:
		decays.push_back({
			"gamma", 1, 100.0*perCent,
			{ { 1274.5*keV, 100.0*perCent } } });
		// ~90% chance of a positron being produced, which will result in two
		// gamma rays from annihilation.
		decays.push_back({
			"gamma", 2, 90.3*perCent,
			{ { 511*keV, 100.0*perCent } } });
		break;
	case RunType::AmBeGamma:
		decays.push_back({
			"gamma", 1, 100.0*perCent,
			Spectrum("am-241_gamma_spectrum.txt") });
		break;
	case RunType::AmBeCaptureGamma:
		decays.push_back({
			"gamma", 1, 100.0*perCent,
			{ { 4.438*MeV, 100.0*perCent } } });
		break;
	case RunType::AmBeNeutron:
		decays.push_back({
			"neutron", 1, 100.0*perCent,
			Spectrum("am-be_neutron_spectrum.txt") });
		break;
	case RunType::DDNeutron:
		decays.push_back({
			"neutron", 1, 100.0*perCent,
			{{ 2.5*MeV, 100.0*perCent }}});
		break;
	case RunType::BackgroundNeutron:
		decays.push_back({
			"neutron", 1, 100.0*perCent,
			Spectrum("background_neutron_spectrum.txt") });
	default:
		break;
	}
	return decays;
}


bool shield::RunTypeIsBackground(RunType runType) {
	switch (runType) {
	case RunType::BackgroundNeutron:
	case RunType::BackgroundGamma:
		return true;
	default:
		return false;
	}
}

