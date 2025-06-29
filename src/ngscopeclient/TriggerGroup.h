/***********************************************************************************************************************
*                                                                                                                      *
* ngscopeclient                                                                                                        *
*                                                                                                                      *
* Copyright (c) 2012-2025 Andrew D. Zonenberg and contributors                                                         *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

/**
	@file
	@author Andrew D. Zonenberg
	@brief Declaration of TriggerGroup
 */
#ifndef TriggerGroup_h
#define TriggerGroup_h

#include "../../lib/scopehal/PausableFilter.h"

/**
	@brief A trigger group is a set of oscilloscopes that all trigger in lock-step

	One instrument, designated "primary", is used as the trigger reference for all other scopes in the group
	("secondaries").

	Mandatory external connection:
		Trigger out of primary to trigger in of each secondary
		Cable lengths need not be matched, the dewskew wizard will measure and calibrate out the trigger path delay

	Strongly recommended external connection:
		Common reference clock supplied to all instruments in the group
		If instruments do not share a common clock, drift will worsen with increasing capture depth
 */
class TriggerGroup : public SerializableObject
{
public:
	enum TriggerType
	{
		TRIGGER_TYPE_SINGLE,
		TRIGGER_TYPE_FORCED,
		TRIGGER_TYPE_AUTO,
		TRIGGER_TYPE_NORMAL
	};

	TriggerGroup(std::shared_ptr<Oscilloscope> primary, Session* session);
	virtual ~TriggerGroup();

	void RemoveScope(std::shared_ptr<Oscilloscope> scope);
	void RemoveFilter(PausableFilter* f);

	void MakePrimary(std::shared_ptr<Oscilloscope> scope);
	void AddSecondary(std::shared_ptr<Oscilloscope> scope);
	void AddFilter(PausableFilter* f);

	void Arm(TriggerType type);
	void Stop();
	bool CheckForPendingWaveforms();
	void DownloadWaveforms();
	void RearmIfMultiScope();

	bool empty()
	{ return m_secondaries.empty() && (m_primary == nullptr) && m_filters.empty(); }

	bool HasScopes()
	{ return !m_secondaries.empty() || (m_primary != nullptr); }

	bool HasSecondaries()
	{ return !m_secondaries.empty(); }

	std::shared_ptr<Oscilloscope> m_primary;
	std::vector<std::shared_ptr<Oscilloscope>> m_secondaries;

	std::vector<PausableFilter*> m_filters;

	std::string GetDescription();

	///@brief True if we should be activated when the start/stop toolbar button is clicked
	bool m_default;

protected:
	void DetachAllWaveforms(std::shared_ptr<Oscilloscope> scope);

	Session* m_session;

	///@brief True if we have multiple scopes and are in normal trigger mode
	bool m_multiScopeFreeRun;
};

#endif
