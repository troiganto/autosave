# Frequently Asked Questions

This file might be edited to add at later points.

##### Table of Contents

* [General Usage](#general-usage)
  * [Does AutoSave also work on Linux or Mac?](#does-autosave-also-work-on-linux-or-mac)
* [Window Matching](#window-matching)
  * [I want to use AutoSave with &lt;Art Program&gt;, but the window doesn't have a title bar.](#i-want-to-use-autosave-with-art-program-but-the-window-doesnt-have-a-title-bar)
  * [AutoSave doesn't work with &lt;Adobe Product&gt;.](#autosave-doesnt-work-with-adobe-product)
  * [I want to use AutoSave with two different art programs, how do?](#i-want-to-use-autosave-with-two-different-art-programs-how-do)
  * [What does "This is a regular expression" mean?](#what-does-this-is-a-regular-expression-mean)
* [Connected Shortcuts](#connected-shortcuts)
  * [When I open a Connected Shortcut, does AutoSave still match windows by caption?](#when-i-open-a-connected-shortcut-does-autosave-still-match-windows-by-caption)
  * [How do the settings in the "Create a Connected Shortcut" window work?](#how-do-the-settings-in-the-create-a-connected-shortcut-window-work)
  * [How can I create a Connected Shortcut without Drag & Drop?](#how-can-i-create-a-connected-shortcut-without-drag--drop)
  * [Can I pin a Connected Shortcut to the taskbar?](#can-i-pin-a-connected-shortcut-to-the-taskbar)
* [Uninstalling](#uninstalling)
  * [AutoSave doesn't find all Connected Shortcuts.](#autosave-doesnt-find-all-connected-shortcuts)
  * [I only want to revert some of the Connected Shortcuts.](#i-only-want-to-revert-some-of-the-connected-shortcuts)
  * [I have several copies of AutoSave on my computer. How does uninstalling work?](#i-have-several-copies-of-autosave-on-my-computer-how-does-uninstalling-work)


## General Usage

When you start AutoSave for the first time, it will automatically open its
settings window. There, you can specify which programs to interact with and
generally modify its settings.

Once you click OK, AutoSave minimizes into the notification area and pick up
its work.
Also in the settings window, you can tell AutoSave how silent to be when its
in the background.

Although it isn't really necessary to shut down AutoSave if you don't use it,
you can always do so by right-clicking its notification icon and
choosing **Shutdown AutoSave**.

If you want AutoSave to always start when you log in to Windows, you can
create an auto-start entry on the **More options** page of the settings window.

If you want AutoSave to always start when you open a specific application,
you can create a [Connected Shortcut](#connected-shortcuts).


#### Does AutoSave also work on Linux or Mac?

No. Unfortunately, AutoSave is a Windows-based application.

Using Wine, AutoSave crashed more often than not, so that won't help either.
(last checked: July 2014)



## Window Matching

When its running, AutoSave will wait silently until a given time
(5 minutes by default) has passed.

If this time has passed, it will look at the window that's currently active.
If this window's title contains a certain *phrase*, AutoSave will simulate
you pressing ctrl+s on your keyboard, which usually makes the active application
save your current progress.

This *phrase* by which AutoSave filters windows is what you enter on the
**Target windows** page of the settings window.


#### I want to use AutoSave with &lt;Art Program&gt;, but the window doesn't have a title bar.

The windows of some modern applications merge don't have a title bar like
most other applications. They might merge it with a menu bar, a tab bar,
or do completely without it.

The **Target windows** page of the settings window contains, at the very bottom,
a line saying *Title of the window under the mouse cursor*.
Below it, you'll find the title of any window you point at using the mouse,
even if the window doesn't have a visible title bar.



#### AutoSave doesn't work with &lt;Adobe Product&gt;!

Various apps from the Adobe Creative suite have caused some confusion during
the beta test of AutoSave.
Let's take, for the sake of an example, Adobe Photoshop.

When there is no file opened in Photoshop, its (invisible) title will be
*Adobe Photoshop*.
Once you open a file, however, the title will change to that file's name,
e.g. *test file.psd*.

This means, for AutoSave to work with such a program, you should specify
the file-type extension on AutoSave's **Target windows** page.
For example, if you want to use AutoSave with Adobe Photoshop,
you'd write ``.psd``, including the period.

Another solution would be using Connected Shortcuts.



#### I want to use AutoSave with two different art programs, how do?

You might have two art programs with very different titles and want to
use AutoSave with both of them.

In this case, you can go to the **Target windows** page of the settings window
and check the box named *This is a regular expression*.
Then, you type both window titles in the proper text box, and separate them
with a pipe character.
For example, if you want to use AutoSave both with GIMP and Microsoft Paint,
you'd type ``paint|gimp``.

Keep in mind that, when the regular expressions checkbox is checked, some
characters (e.g. the period) have a special meaning.
If you want to use them literally, you have to write a backslash in front
of them.

For example, if you want to use AutoSave both with GIMP and Adobe Photoshop,
you'd check *This is a regular expression* and type ``\.psd|gimp`` into
the text box above.
Note how there is a backslash in front of the period; if it weren't, the period
would mean *absolutely every character may be in this place*.


#### What does "This is a regular expression" mean?

Regular Expressions ("regex" for short) are a kind of mini-language with which
you can specify certain *patterns* that you are searching for in a text.

AutoSave can use regular expressions for title matching if a regular old
"the title must contain this string of letters" doesn't do it for you.

The [Wikipedia](http://en.wikipedia.org/wiki/Regular_expression#POSIX_basic_and_extended)
and the [rest of the Internet](https://duckduckgo.com/?q=regular+expressions+tutorial)
contain many tutorials surrounding regular expressions, but what most people
would need when using AutoSave is:

* ``gimp|sai -|paint`` matches a window if its title contains ``gimp``, ``sai -`` or ``paint``;
* ``you (are|were)`` matches both ``you are`` and ``you were``;
* ``[0-9]`` matches any digit;
* ``[0-9]{1,3}`` matches any number that contains 1, 2, or 3 digits.



## Connected Shortcuts

A Connected Shortcut is a desktop shortcut file that opens both AutoSave
and an application of your choice at the same time.
AutoSave will then only target this one application and will automatically
shut down when you close this application.

#### When I open a Connected Shortcut, does AutoSave still match windows by caption?

No. When you click a Connected Shortcut, AutoSave will target only the app
that you started it with.
This mode is completely independent from the normal mode (where AutoSave
targets windows by caption) and it uses a different mechanism.


#### How do the settings in the "Create a Connected Shortcut" window work?

They overwrite the normal settings.

Suppose you create a Connected Shortcut and specify a custom interval.
When you start AutoSave with this Connected Shortcut, all its settings will
be as if you started it normally -- except of the interval, which will be as
specified when creating the Connected Shortcut.

Keep in mind that if you use custom settings in a Connected Shortcut, you
cannot easily change them afterwards. You'd have to create a new Connected
Shortcut with new, different custom settings.


#### How can I create a Connected Shortcut without Drag & Drop?

There are various reasons why a user could be unable to drag an application
into or out of the **Create a Connected Shortcut** dialog.

If this is the case, you can double-click the boxes in the dialog to open
an **Open** or **Save** dialog respectively.

You can also use the ``Tab`` key on your keyboard to focus these boxes and
press ``Enter`` for the same effect.


#### Can I pin a Connected Shortcut to the taskbar?

Yes, but they won't work very well.
Because of the way Connected Shortcuts work, you can expect the connected
art program to open in a new taskbar item instead of using the shortcut's one.


## Uninstalling

AutoSave saves its settings and other things, like Connected Shortcuts or
and auto-start entry, on your harddrive.
Although the disk space these changes occupy on your disk are negligibly small,
you should delete them if you want to remove AutoSave from your system.

To do that, go to the **Remove & Uninstall** page of AutoSave's
settings window and click the proper button.
Follow the wizard and, when it's completed, delete your copy of ``AutoSave.exe``.

#### AutoSave doesn't find all Connected Shortcuts.

The **Revert Connected Shortcuts** page of the AutoSave Uninstall wizard
contains a list of automatically found Connected Shortcuts.
There might be cases where it doesn't find all shortcuts on your system.

If that's the case, you can drag a Connected Shortcut into this list and drop
it there.
If AutoSave recognizes it as a Connected Shortcut, it will be added to the list.


#### I only want to revert some of the Connected Shortcuts.

The **Revert Connected Shortcuts** page of the AutoSave Uninstall wizard
contains a list of automatically found Connected Shortcuts.

To remove a file from this list, select it and press the ``Del`` key.


#### I have several copies of AutoSave on my computer. How does uninstalling work?

AutoSave saves all its settings on a per-user basis.
If there are multiple users on your computer (i.e. multiple user *acounts*)
using the same ``AutoSave.exe`` file, everyone of them has to delete their
settings before the exe file should be deleted.

Because of this, it might be advisable for each user account to have their
own copy of AutoSave.
