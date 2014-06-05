// MiscSettings.h : Contains all settings that don't concern window matching:
// Sending interval, sent keyboard input, and verbosity.
// Member functions only throw if CommandLineParser throws.
// Beware: toCommandLine may also throw std::invalid_argument or std::out_of_range!

#pragma once

#include "stdafx.h"
#include "CommandLineParser.h"

using std::wstring;

class MiscSettings
{
public:
	enum AttributesMask
	{
		ATT_NONE = 0,
		ATT_INTERVAL = 0x1,
		ATT_HOTKEY = 0x2,
		ATT_VERBOSITY = 0x4,
		ATT_ALL = ATT_INTERVAL | ATT_HOTKEY | ATT_VERBOSITY
	};

	enum Verbosity {
		MIN_VERBOSITY,
		QUIET = MIN_VERBOSITY,
		SHOW_ICONS,
		ALERT_START,
		ALERT_FIVE_SECONDS,
		MAX_VERBOSITY = ALERT_FIVE_SECONDS
	};

public:
	MiscSettings();
	~MiscSettings();

	bool operator==(const MiscSettings& cfg) const;
	bool operator!=(const MiscSettings& cfg) const;

	// Configuration and command line
	void loadFromCommandLine(const CommandLineParser& cli);
	wstring toCommandLine(int attributesMask) const;
	inline static const wchar_t* getAllowedKeys() { return L"HIV"; }

	// Getters and Setters

	inline WORD getHotkey() const { return m_hotkey; }
	inline void setHotkey(WORD hotkey) { m_hotkey = hotkey; }

	inline static UINT getMinInterval() { return 10; }
	inline static UINT getMaxInterval() { return 24 * 60 * 60; }

	inline UINT getInterval() const  { return m_interval; }
	inline void setInterval(UINT interval) {
		m_interval = __min(__max(interval,
			getMinInterval()), getMaxInterval());
	}

	inline Verbosity getVerbosity() const { return m_verbosity; }
	inline bool verbosityExceeds(Verbosity minVerbosity) const {
		return m_verbosity >= minVerbosity;
	}
	inline void setVerbosity(Verbosity verbosity) { m_verbosity = verbosity; }
	inline void setVerbosity(int verbosity) {
		m_verbosity = (Verbosity)__min(__max(verbosity,
			MIN_VERBOSITY), MAX_VERBOSITY);
	}


private:
	WORD m_hotkey;
	UINT m_interval;
	Verbosity m_verbosity;

};

