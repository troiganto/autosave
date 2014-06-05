// ConnectedShortcut.h : A Shortcut subclass that can be
// connected to and disconnected from AutoSave.
// On failure, these usually throw OleException.

#pragma once

#include "stdafx.h"
#include "Shortcut.h"
#include "OleUtils.h"
#include "CommandLineParser.h"

using std::wstring;

class ConnectedShortcut : public Shortcut
{
public:
	ConnectedShortcut();
	~ConnectedShortcut();

	// Directly manipulate the shortcut's properties.
	void connect(const wstring& settingsArgs);
	void reconnect(const wstring& settingsArgs);
	void disconnect();
	static void disconnect(const wstring& filePath);
	
	static wstring connectFileName(const wstring& targetPath);
	static wstring disconnectFileName(const wstring& shortcutName);

	// Careful: The non-static version of isConnected may throw,
	// the static version silently returns false on failure.
	bool isConnected();
	static bool isConnected(const wstring& shortcutPath);

	// Careful: Results of isConnected are cached for each object.
	inline void clearConnectedCache() { m_hasIsConnectedBeenCached = false; }

	// With these functions, you may override what ConnectedShortcut
	// considers to be AutoSave. Only useful for unit-testing.
	inline static wstring getSelfPath() {
		return m_customSelfPath.empty() ?
			OleUtils::getSelfPath() : m_customSelfPath;
	}
	inline static void setSelfPath(const wstring& selfPath) {
		m_customSelfPath = selfPath;
	}

protected:
	static wstring m_customSelfPath;

	bool computeIsConnected() const;
	bool m_isConnectedCachedResult;
	bool m_hasIsConnectedBeenCached;

	vector<wstring> getLArgs() const;
	static bool startsWith(const wstring& tested, const wstring& prefix);
};
