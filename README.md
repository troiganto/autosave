Broken AutoSave
========

A Visual C++ application for Windows that regularly simulates pressing ```Ctrl+S``` for other applications.

Through this approach, you can add an auto-save feature to virtually every application, as long as it has a normal application window.

## Using Broken AutoSave

### General Workflow

AutoSave usually minimizes into the notification area.
After the specified amount of time has passed (5 minutes by default), AutoSave checks whether the active window matches a specified filter. (see below for more info)
If the active window doesn't match, AutoSave waits until a matching window gets activated.
If AutoSave detects that none of the open windows matches its filter, it resets its timer back to the beginning.

If the timer is at zero *and* the active window matches, AutoSave simulates the keystrokes specified in its configuration (```Ctrl+S``` by default.) It does so using the [SendInput](http://msdn.microsoft.com/en-us/library/windows/desktop/ms646310%28v=vs.85%29.aspx) function.

Right-clicking on AutoSave's notification icon allows the user to temporarily disable AutoSave, to shut it down, and to open the options window.
The options window gives access to the configuration and all additional tools.

### Targeting Windows

AutoSave only sends keyboard input, if a the active window matches a given filter. There are two ways to specify which windows to target:
* Caption matching (by normally starting AutoSave),
* Connecting to another application (by using Connected Shortcuts).

#### Caption matching

By default, AutoSave lets the user specify a phrase to filter windows by. AutoSave then targets a window only, if the window's caption *contains* the given phrase. This comparison is case-insensitive.

This means that if the user specifies "gimp" as the filter phrase, then AutoSave simulates pressing ```Ctrl+S``` as soon as the timer is up *and* the active window's caption contains "gimp".
This could be the main window of [GIMP](http://gimp.org), but it could also be a folder names "gimp" being open in the Windows Explorer.
It's up to the user to enter an unambiguous phrase.

Additionally, AutoSave allows the user to filter windows using [regular expressions](http://simple.wikipedia.org/wiki/Regular_expression) instead of normal text comparison.
These regular expressions follow the [ECMAScript syntax](http://www.cplusplus.com/reference/regex/ECMAScript/).
The regex matching is case-insensitive as well.

#### Connecting to Another Application (Connected Shortcuts)

AutoSave accepts command-line arguments.
If you pass another document or ```*.exe``` file to AutoSave, it will open that document or start that application, respectively.
AutoSave will ignore its filter phrase in this case and *only* target windows of the started application.

If connected to another application, AutoSave filters windows based on their process IDs.
This means that connecting may fail if, for example, opening a document doesn't start a new process but rather opens a new tab in another process.

AutoSave allows the user to create *Connected Shortcuts*.
These are normal Windows shortcut files which open AutoSave together with an application or document of their choice. (using the mechanism described above)

### Auto-start

As with any other application, the user can put AutoSave or a shortcut to it into your start-up directory.
This allows AutoSave to be started as soon as the Windows session has started.

### Uninstall

AutoSave does the following changes on the user's machine:
1. It saves its configuration in the user's registry (the location is ```HKEY_CURRENT_USER\Software\Broken AutoSave```);
2. It may create a shortcut in the start-up directory if the user asks it to;
3. The user may create Connected Shortcuts with it.

(The x86 version may create temporary files in the user's ```%TEMP%``` directory.
But this is hardly important.)

AutoSave allows the user to revert all these changes from within the options window.
Each of these three steps may be performed separately.

Concerning the Connected Shortcuts, AutoSave lets the user choose whether they should be deleted or converted into normal shortcuts.
The latter would convert a shortcut that executes the line ```C:\path\to\autosave.exe C:\path\to\another\program.exe``` into a shortcut that executes ```C:\path\to\another\program.exe```.

## Building Broken AutoSave

Broken AutoSave is a native C++ project written with Visual Studio Express 2013.
The necessary project files are included in the repository, so it shouldn't be difficult to compile the codebase.

The repository contains the following sub-folders:
* AutoSave: Project folder containing the trimmed-down main project. Contains only the ```main``` function and nothing else. Everything else has been outsourced.
* AutoSave_libs: Project folder containing the actual program. All classes and namespaces are declared in this folder. The ```AutoSave_libs.vcxproj.filters``` file brings a bit of order into this pile of code.
* AutoSave_tests: Project folder for unit-tests. Tests are written for Visual Studio's built-in test framework.
* AutoSave Test Files: Contains a few files the unit tests perform tests on.
* AutoSave Icons: Contains the ```*.ico``` files for AutoSave. They are provided as SVG files. This folder also contains a quick and dirty Python script that builds the correct ICO files. This script depends on Inkscape and ImageMagick.
* Map: A diagram showing the relations between the most important classes and namespaces in AutoSave. It disregards the "utility" classes since these would clutter it up considerably. The diagram looks a bit like a UML class diagram but disregards most requirements.


