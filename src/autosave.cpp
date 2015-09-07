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

#include <autosave.hpp>
#include <core/Communicator.hpp>

#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    using namespace std;

    std::string firefox("/usr/lib64/firefox/firefox");
    core::Communicator com;

    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
        core::Process proc = com.get_active_process();
        //~ if (com.get_active_process().started_by(firefox)) {
            //~ cout << "\u2713";
        //~ }
        //~ else {
            //~ cout << "\u2717";
        //~ }
        cout.flush();
    }

    return 0;
}
