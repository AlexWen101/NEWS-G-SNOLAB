#ifndef __SHIELD_SPECTRUM_H_
#define __SHIELD_SPECTRUM_H_

#include <initializer_list>
#include <string>
#include <vector>

#include <G4Types.hh>

namespace shield {

struct EnergyFrequencyPair {
	
	G4double energy;
	G4double frequency;
	
};

class Spectrum {
	
public:
	
	Spectrum(std::initializer_list<EnergyFrequencyPair> spectrum);
	Spectrum(std::string fileName);
	virtual ~Spectrum() {
	}
	
	// Given a number between 0 and 1, returns an value from the spectrum.
	G4double Draw(G4double x);
	
private:
	
	std::vector<EnergyFrequencyPair> m_spectrum;
};

}

#endif

