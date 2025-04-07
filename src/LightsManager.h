#ifndef LightsManager_H
#define LightsManager_H

#include "PlayerNumber.h"
#include "GameInput.h"
#include "EnumHelper.h"
#include "Preference.h"
#include "RageTimer.h"

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

extern Preference<float>	g_fLightsFalloffSeconds;
extern Preference<float>	g_fLightsAheadSeconds;

enum CabinetLight
{
	LIGHT_MARQUEE_UP_LEFT,
	LIGHT_MARQUEE_UP_RIGHT,
	LIGHT_MARQUEE_LR_LEFT,
	LIGHT_MARQUEE_LR_RIGHT,
	LIGHT_BASS_LEFT,
	LIGHT_BASS_RIGHT,
	NUM_CabinetLight,
	CabinetLight_Invalid
};
/** @brief Loop through each CabinetLight on the machine. */
#define FOREACH_CabinetLight( i ) FOREACH_ENUM( CabinetLight, i )
const RString& CabinetLightToString( CabinetLight cl );
CabinetLight StringToCabinetLight( const RString& s);

enum LightsMode
{
	LIGHTSMODE_ATTRACT,
	LIGHTSMODE_JOINING,
	LIGHTSMODE_MENU_START_ONLY,
	LIGHTSMODE_MENU_START_AND_DIRECTIONS,
	LIGHTSMODE_DEMONSTRATION,
	LIGHTSMODE_GAMEPLAY,
	LIGHTSMODE_STAGE,
	LIGHTSMODE_ALL_CLEARED,
	LIGHTSMODE_TEST_AUTO_CYCLE,
	LIGHTSMODE_TEST_MANUAL_CYCLE,
	NUM_LightsMode,
	LightsMode_Invalid
};
const RString& LightsModeToString( LightsMode lm );
LuaDeclareType( LightsMode );

struct LightsState
{
	bool m_bCabinetLights[NUM_CabinetLight];
	bool m_bGameButtonLights[NUM_GameController][NUM_GameButton];

	// This isn't actually a light, but it's typically implemented in the same way.
	bool m_bCoinCounter;
};

// Define a type for light state update requests
using LightRequest = std::function<void()>;

// Thread-safe queue for light requests
class LightRequestQueue {
public:
    void Push(LightRequest request) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_requestPending) {
            // Skip adding the request if one is already pending
            return;
        }
        m_queue.push(request);
        m_requestPending = true; // Mark that a request is pending
        m_condition.notify_one();
    }

    LightRequest Pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this] { return !m_queue.empty() || m_stop; });
        if (m_queue.empty()) return nullptr;
        LightRequest request = m_queue.front();
        m_queue.pop();
        if (m_queue.empty()) {
            m_requestPending = false; // Mark that no request is pending
        }
        return request;
    }

    void Stop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
        m_condition.notify_all();
    }

private:
    std::queue<LightRequest> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    bool m_stop = false;
    bool m_requestPending = false; // Tracks if a request is pending
};

class LightsDriver;
/** @brief Control lights. */
class LightsManager
{
public:
	LightsManager();
	~LightsManager();

	void Update( float fDeltaTime );
	bool IsEnabled() const;

	void BlinkCabinetLight( CabinetLight cl );
	void BlinkGameButton( GameInput gi );
	void BlinkActorLight( CabinetLight cl );
	void TurnOffAllLights();
	void PulseCoinCounter() { ++m_iQueuedCoinCounterPulses; }
	float GetActorLightLatencySeconds() const;

	void SetLightsMode( LightsMode lm );
	LightsMode GetLightsMode();

	void PrevTestCabinetLight()		{ ChangeTestCabinetLight(-1); }
	void NextTestCabinetLight()		{ ChangeTestCabinetLight(+1); }
	void PrevTestGameButtonLight()	{ ChangeTestGameButtonLight(-1); }
	void NextTestGameButtonLight()	{ ChangeTestGameButtonLight(+1); }

	CabinetLight	GetFirstLitCabinetLight();
	GameInput	GetFirstLitGameButtonLight();

	void SetLightsState();
    void ResetLightsState();

private:
	void ChangeTestCabinetLight( int iDir );
	void ChangeTestGameButtonLight( int iDir );

	float m_fSecsLeftInCabinetLightBlink[NUM_CabinetLight];
	float m_fSecsLeftInGameButtonBlink[NUM_GameController][NUM_GameButton];
	float m_fActorLights[NUM_CabinetLight];	// current "power" of each actor light
	float m_fSecsLeftInActorLightBlink[NUM_CabinetLight];	// duration to "power" an actor light

	std::vector<LightsDriver*> m_vpDrivers;
	LightsMode m_LightsMode;
	LightsState m_LightsState;

	int m_iQueuedCoinCounterPulses;
	RageTimer m_CoinCounterTimer;

	int GetTestAutoCycleCurrentIndex() { return (int)m_fTestAutoCycleCurrentIndex; }

	float			m_fTestAutoCycleCurrentIndex;
	CabinetLight	m_clTestManualCycleCurrent;
	int				m_iControllerTestManualCycleCurrent;

	LightRequestQueue m_requestQueue;
	std::thread m_workerThread;
	std::atomic<bool> m_stopWorker;

	void ProcessLightRequests();
};

extern LightsManager*	LIGHTSMAN;	// global and accessible from anywhere in our program

#endif

/*
 * (c) 2003-2004 Chris Danford
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
