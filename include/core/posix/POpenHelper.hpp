/*
 * POpenHelper.hpp
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

#pragma once

#include <string>

// TODO: Add killing capability (http://stackoverflow.com/questions/548063)


namespace core
{
    /*!A wrapper class around the POSIX functions `popen`/`pclose`.
     *
     * This class wraps calls to `popen` and `pclose` in an RAII manner.
     * The output of the child may be retrieved.
     */
    class POpenHelper
    {
    public:
        /*!Create an instance not bound to any process.
         */
        POpenHelper() noexcept;

        /*!Constructs an instance and immediately calls open().
         *
         * \sa open()
         */
        POpenHelper(const std::string& cmdline);

        /*!Calls close() before destroying the instance.
         *
         * \sa close()
         */
        ~POpenHelper();

        /*!Start a new process and associate its `stdout` with this object.
         *
         * \param cmdline The command line to execute. It is passed
         *                directly to \c popen().
         * \throws std::system_error if \c popen() fails. Its error code
         *         is the \c errno set by \c popen().
         */
        void open(const std::string& cmdline);

        /*!Wait for the associated process to finish and return exit code.
         *
         * If there is a child process associated with this object, wait
         * until it has exited and return its exit code.
         * If no process is associated with this object, do nothing and
         * return 0.
         *
         * \returns The exit code of the associated process or 0 if no
         *          process is associated with this object.
         */
        int close() noexcept;

        /*!Read the output of the associated process and return it as string.
         *
         * \returns The output of the child process.
         * \throws std::invalid_argument if no process is associated with
         *         this object.
         */
        std::string get_output();

    private:
        std::FILE* m_out;
    };
}
