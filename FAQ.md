Frequently Asked Questions
========

This file might be edited to add at later points.



## Q: I typed "Photoshop" into the text box, but the Photoshop window doesn't appear in the list.

**A:** This is because Photoshop's window title doesn't actually contain the word "Photoshop".
Use the filter phrase ".psd" instead.

The "Target Windows" page of AutoSave's options window contains a little tool for windows with an invisible title bar.
Whenever you point at a window with your mouse, the bottom-most text line of the form will show you that window's title.

You can use this tool to find out what to enter in the top text box.



## Q: Is it possible that AutoSave starts whenever I open PSD files?

**A:** Not without screwing around with your system in unethical ways.
Use title matching instead.
If you enable regular expressions, the phrase "gimp|paint" matches all windows whose titles contain "gimp" or "paint".

Windows remembers file type associations in a global database called registry.
Every applications registers an entry for each file type it wants to be associated with.
If you wanted to start AutoSave whenever you open, say, a PSD file, AutoSave would have to manipulate these entries that belong to other applications.

That's as if, say, Photoshop was working in an office and was happy that it just completed all its paperwork, and suddenly, AutoSave would jump in through the window, grab all of Photoshop's forms, strike out "Photoshop" on each of them and scribble "AutoSave" on them instead in small, ugly letters.

It's a bit intrusive.
