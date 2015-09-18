/*
 * autosave.cpp
 *
 * Copyright 2015 Nico <nico@FARD>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "autosave.hpp"
#include "core/Thread.hpp"
#include "core/Settings.hpp"

#include <iostream>
#include <chrono>

int main()
{
    using namespace std;

    string input;
    core::Settings settings;
    settings.set_interval(std::chrono::seconds(30));
    cout << "Before Thread started" << endl;
    {
        core::Thread thread(settings);
        thread.start();
        cout << "After Thread started" << endl;
        getline(cin, input);
    }
    cout << "After Thread stopped" << endl;
    cout << "\"" << input << "\"" << endl;
    return 0;
}
