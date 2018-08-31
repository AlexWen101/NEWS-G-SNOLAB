#ifndef __SHIELD_ACCUMULABLE_LIST_H_
#define __SHIELD_ACCUMULABLE_LIST_H_

#include <vector>

#include <G4VAccumulable.hh>
#include <G4String.hh>

namespace shield {

template<typename T>
class AccumulableList : public G4VAccumulable {
	
public:
	
	AccumulableList() : m_list() {
	}
	
	virtual ~AccumulableList() {
	}
	
	void Add(T value) {
		m_list.push_back(value);
	}
	
	T const* Begin() const {
		if (m_list.empty()) {
			return NULL;
		}
		else {
			return &(*m_list.begin());
		}
	}
	
	T const* End() const {
		if (m_list.empty()) {
			return NULL;
		}
		else {
			return &(*m_list.end());
		}
	}
	
	unsigned int Size() const {
		return m_list.size();
	}
	
	virtual void Merge(G4VAccumulable const& other) {
		AccumulableList<T> const& accumulableList =
			static_cast<AccumulableList<T> const&>(other);
		m_list.insert(
			m_list.end(),
			accumulableList.m_list.begin(),
			accumulableList.m_list.end());
	}
	
	virtual void Reset() {
		m_list.clear();
	}
	
private:
	
	std::vector<T> m_list;
};

}

#endif

