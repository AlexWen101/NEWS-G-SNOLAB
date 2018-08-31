#include "spectrum.h"

#include <limits>
#include <fstream>

#include <G4SystemOfUnits.hh>

using namespace shield;

Spectrum::Spectrum(std::initializer_list<EnergyFrequencyPair> spectrum) :
		m_spectrum(spectrum) {
	G4double netFrequency = 0.0;
	for (EnergyFrequencyPair pair : m_spectrum) {
		netFrequency += pair.frequency;
	}
	for (EnergyFrequencyPair& pair : m_spectrum) {
		pair.frequency /= netFrequency;
	}
}

Spectrum::Spectrum(std::string fileName) {
	std::fstream file(fileName, std::ios::in);
	// Skip the first line.
	file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	G4double netFrequency = 0.0;
	while (file.good()) {
		EnergyFrequencyPair nextPair;
		double energy, frequency;
		file >> energy;
		file >> frequency;
		nextPair.energy = energy*MeV;
		nextPair.frequency = frequency;
		netFrequency += frequency;
		m_spectrum.push_back(nextPair);
	}
	file.close();
	
	for (EnergyFrequencyPair& pair : m_spectrum) {
		pair.frequency /= netFrequency;
	}
}

G4double Spectrum::Draw(G4double x) {
	for (EnergyFrequencyPair pair : m_spectrum) {
		if (x < pair.frequency) {
			return pair.energy;
		}
		else {
			x -= pair.frequency;
		}
	}
	return m_spectrum.back().energy;
}

