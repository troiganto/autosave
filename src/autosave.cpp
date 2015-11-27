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

using namespace std;
using namespace std::literals;

void mainloop(const core::Settings& settings)
{
    core::Thread thread(settings);
    thread.start();
    this_thread::sleep_for(12s);
}


int main()
{
    using namespace std;

    core::Settings settings;
    settings.interval(std::chrono::seconds(6));
    settings.key_combo(core::KeyCombo('a'));
    settings.target_apps().push_back("/usr/bin/geany");
    mainloop(settings);
    return 0;
}
