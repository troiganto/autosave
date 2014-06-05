#include "stdafx.h"
#include "ConnectedShortcut.h"



wstring ConnectedShortcut::m_customSelfPath;

ConnectedShortcut::ConnectedShortcut()
	: m_isConnectedCachedResult(false),
	  m_hasIsConnectedBeenCached(false)
{
}



ConnectedShortcut::~ConnectedShortcut()
{
}



void ConnectedShortcut::connect(const wstring& settingsArgs)
{
	if (isConnected())
		return;
	
	wstring args = getArguments();
	wstring target = CommandLineParser::escapeArgument(getPath());
	wstring description = SHORT_APP_NAME L" + " + getDescription();

	if (!args.empty())
		target.push_back(L' ');
	if (!settingsArgs.empty() && settingsArgs.back() != L' ')
		target.insert(target.begin(), L' ');

	m_pLink->SetPath(getSelfPath().data());
	m_pLink->SetArguments((settingsArgs + target + args).data());
	m_pLink->SetDescription(description.data());

	m_isConnectedCachedResult = true;
}



void ConnectedShortcut::reconnect(const wstring& settingsArgs)
{
	if (!isConnected())
		return;

	wstring largs = CommandLineParser::joinArguments(getLArgs());
	if (!settingsArgs.empty() && settingsArgs.back() != L' ')
		largs.insert(largs.begin(), L' ');

	m_pLink->SetArguments((settingsArgs + largs).data());
}



void ConnectedShortcut::disconnect()
{
	if (!isConnected())
		return;

	vector<wstring> args = getLArgs();
	wstring newCommandLine = CommandLineParser::joinArguments(
		args.begin() + 1, args.end());
	wstring description = getDescription();
	wstring prefix = SHORT_APP_NAME L" + ";
	if (description.substr(0, prefix.length()) == prefix)
		description.erase(0, prefix.length());
	

	m_pLink->SetPath(args[0].data());
	m_pLink->SetArguments(newCommandLine.data());
	m_pLink->SetDescription(description.data());

	m_isConnectedCachedResult = false;
}



void ConnectedShortcut::disconnect(const wstring& filePath)
{
	ConnectedShortcut csc;
	csc.load(filePath, STGM_READWRITE);
	csc.disconnect();
	csc.save(filePath, FALSE);
}



bool ConnectedShortcut::isConnected()
{
	// A shortcut is a connected shortcut if and only if:
	//     (1) it points to AutoSave and
	//     (2) its command line arguments consist of:
	//         (2a) valid key-word arguments
	//         (2b) one or more list arguments.
	if (!m_hasIsConnectedBeenCached)
		m_isConnectedCachedResult = computeIsConnected();

	return m_isConnectedCachedResult;
}



bool ConnectedShortcut::isConnected(const wstring& filePath)
{
	// Shortcut: Don't try non-link files.
	if (PathMatchSpec(filePath.data(), L"*.lnk") == FALSE)
		return false;
	try {
		ConnectedShortcut csc;
		csc.load(filePath, STGM_READ);
		return csc.isConnected();
	}
	catch (std::exception&) {
		return false;
	}
}



bool ConnectedShortcut::computeIsConnected() const
{
	return !getLArgs().empty() &&
		(_tcsicmp(getSelfPath().data(), getPath().data()) == 0);
}



wstring ConnectedShortcut::connectFileName(const wstring& targetPath)
{
	return OleUtils::getFileDisplayName(targetPath) +
		L" + " SHORT_APP_NAME L".lnk";;
}



wstring ConnectedShortcut::disconnectFileName(const wstring& shortcutName)
{
	const wstring suffix = L" + " SHORT_APP_NAME L".lnk";
	if (PathMatchSpec(shortcutName.data(), (L"*" + suffix).data()) == FALSE)
		return shortcutName;

	wstring newName = shortcutName;
	newName.erase(newName.length() - suffix.length());
	newName.append(L".lnk");
	return newName;
}



vector<wstring> ConnectedShortcut::getLArgs() const
{
	CommandLineParser cli;
	cli.allowAllKeys();
	try {
		cli.parse(getArguments());
		return cli.getLArgs();
	}
	catch (CLIException&) {
		return {};
	}
}



bool ConnectedShortcut::startsWith(const wstring& tested, const wstring& prefix)
{
	return (tested.length() >= prefix.length()) &&
		(std::mismatch(prefix.cbegin(), prefix.cend(), tested.cbegin()).first == prefix.cend());
}






