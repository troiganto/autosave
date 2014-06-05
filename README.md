Broken AutoSave
========

A Visual C++ application for Windows that regularly simulates pressing ```Ctrl+S``` for other applications.

Through this approach, you can add an auto-save feature to virtually every application, as long as it has a normal application window.

## Using Broken AutoSave

### General Workflow

AutoSave usually minimizes into the notification area.
After the specified amount of time has passed (5 minutes by default), AutoSave checks whether a target window is currently active.
If no target window is active, AutoSave waits until one gets activated.
If AutoSave detects that no target window is open at all, it resets its timer back to the beginning.

If the timer is at zero *and* a target window currently has the focus, AutoSave simulates the keystrokes specified in the configuration (```Ctrl+S``` by default.) It does so using the [SendInput](http://msdn.microsoft.com/en-us/library/windows/desktop/ms646310%28v=vs.85%29.aspx) function.

Right-clicking on the notification icon allows the user to temporarily disable AutoSave, to shut it down, and to open the options window.
The options window gives access to the configuration and all additional tools.

### Target Windows

AutoSave only sends keyboard input, if a target window is active. There are two ways to specify which windows to target:
* Caption matching,
* Connecting to another application.

#### Caption matching

By default, AutoSave lets the user specify a phrase to filter windows by. In this case, AutoSave targets a window if the window's caption *contains* the given phrase. This comparison is case-insensitive.

This means that if the user specifies "gimp" as the filter phrase, then AutoSave simulates pressing ```Ctrl+S``` as soon as the timer is up and a window whose caption contains "gimp" is active.
This could be the main window of [GIMP](http://gimp.org), but it could also be a Windows explorer window showing the contents of a folder named "gimp".
It's up to the user to enter an unambiguous phrase.

Additionally, AutoSave allows the user to filter by [regular expressions](http://simple.wikipedia.org/wiki/Regular_expression) instead of common text comparison.
These regular expressions follow the [ECMAScript syntax](http://www.cplusplus.com/reference/regex/ECMAScript/).
The regex matching is case-insensitive as well.

#### Connecting to Another Application

AutoSave accepts command-line arguments.
If you pass another document or ```*.exe``` file to AutoSave, it will open that document or start that application, respectively.
It will also ignore its filter phrase and *only* target the window of the started application.

If connected to another application, AutoSave targets a window based on its process ID.
This means that connecting may fail if, for example, opening a document doesn't start a new process but rather opens a new tab in another process.

AutoSave allows the user to create *Connected Shortcuts*.
These are normal Windows shortcut files which open AutoSave together with an application or document of your choice.
This is done via the mechanism described above.

### Auto-start

As with any other application, the user can put AutoSave or a shortcut to it into your start-up directory.
This allows AutoSave to be started as soon as the Windows session has started.

### Uninstall

AutoSave does the following changes on the user's machine:
* It saves its configuration in the user's registry (the location is HKEY_CURRENT_USER\Software\Broken AutoSave);
* It may create a shortcut in the start-up directory if the user asks it to;
* The user may create Connected Shortcuts with it.

AutoSave allows the user to revert all these changes from within the options window.
Each of these three steps may be performed separately.
Concerning the Connected Shortcuts, AutoSave lets the user choose whether they should be deleted or converted into normal shortcuts.

## Building Broken AutoSave

Broken AutoSave is a native C++ project written with Visual Studio Express 2013.
The necessary project files are included in the repository, so it shouldn't be difficult to compile the codebase.

The repository contains the following sub-folders:
* AutoSave: Project folder containing the trimmed-down main project. Contains only the ```main``` function and nothing else.
* AutoSave_libs: Project folder containing the actual program. All classes and namespaces are declared in this folder. The ```AutoSave_libs.vcxproj.filters``` file brings a bit of order into this pile of code.
* AutoSave_tests: Project folder for unit-tests. Tests are written for Visual Studio's built-in test framework.
* AutoSave Test Files: Contains a few files the unit tests use to check on AutoSave.
* AutoSave Icons: Contains the ```*.ico``` files for AutoSave. They are provided as SVG files. This folder also contains a quick and dirty Python script that builds the correct ICO files. This script depends on Inkscape and ImageMagick.
* Map: A diagram showing the relations between the most important classes and namespaces in AutoSave. It disregards the "utility" classes since these may be used by any class and including them in that map would make it even less readable than it already is.


