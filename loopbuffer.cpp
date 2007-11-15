/* This file is a part of ponder, a English/American checkers game.

   Copyright (c) 2006, 2007 Mamiyami Information.
                     Gong Jie <neo@mamiyami.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
/** @file loopbuffer.cpp
 *  @brief
 *  $Author: neo $
 *  $Date: 2007-11-15 10:36:31 $
 *  $Revision: 1.14 $
 */

#include <cerrno>
#include <unistd.h>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include "loopbuffer.hpp"

namespace checkers
{
	char loopbuffer::front(void) const
	{
		assert(!this->is_empty());		

		return this->_buffer[this->_front];
	}

	void loopbuffer::pop_front(void)
	{
		assert(!this->is_empty());		

		if ('\n' == this->front())
		{
			--this->_lines;
		}
		this->_front = (this->_front + 1) % this->_max_size;
	}

	bool loopbuffer::read(int fd)
	{
		char c;
		ssize_t n;

		for (;;)
		{
			n = ::read(fd, &c, 1);
			if (n < 0)
			{
				if (EAGAIN == errno || EWOULDBLOCK == errno)
				{
					break;
				}
				std::ostringstream error;
				error << "read() failed while read from fd - " << fd;
				/// @throw std::runtime_error when read() failed.
				throw std::runtime_error(error.str());
			}
			else if (0 == n)
			{
				/// @retval false when read() reach EOF.
				return false;
			}
			else
			{
				this->push_back(c);
			}
		}
		/// @retval true on success.
		return true;
	}

	bool loopbuffer::write(int fd)
	{
		char c;
		ssize_t n;
		while (!this->is_empty())
		{
			c = this->front();
			n = ::write(fd, &c, 1);
			if (n < 0)
			{
				if (EAGAIN == errno || EWOULDBLOCK == errno)
				{
					break;
				}
				std::ostringstream error;
				error << "write() failed while write to fd - " << fd;
				/// @throw std::runtime_error when write() failed.
				throw std::runtime_error(error.str());
			}
			else if (0 == n)
			{
				/** @retval false when write() returns 0.
				 *   This is abnormal, and should not happen.
				 */
				return false;
			}
			else
			{
				this->pop_front();
			}
		}
		/// @retval true on success.
		return true;
	}

	std::string loopbuffer::getline(void)
	{
		char c;
		std::string str;

		if (this->_lines)
		{
			for (;;)
			{
				c = this->front();
				if ('\n' == c)
				{
					this->pop_front();
					break;
				}
				/// @warning This is not exception-safe, may
				/// cause data lose when operator +=() failed.
				str += c;
				this->pop_front();
			}
		}

		return str;
	}

	void loopbuffer::push_back(char c)
	{
		if (this->is_full())
		{
			this->enlarge();
		}

		if ('\n' == c)
		{
			++this->_lines;
		}
		this->_buffer[this->_rear] = c;
		this->_rear = (this->_rear + 1) % this->_max_size;
	}

	void loopbuffer::push_back(const std::string& str)
	{
		for (std::string::const_iterator pos = str.begin();
			pos != str.end(); ++pos)
		{
			this->push_back(*pos);
		}	
	}

	void loopbuffer::push_back(const char* s)
	{
		for (; *s != '\0'; ++s)
		{
			this->push_back(*s);
		}
	}

	// ================================================================

	void loopbuffer::enlarge(void)
	{
		assert(this->is_full());

		const int multiple = 2;
		char* buffer = new char[this->_max_size * multiple];
		if (this->_front < this->_rear)
		{
			memcpy(buffer, this->_buffer + this->_front,
				this->_rear - this->_front);
		}
		else
		{
			memcpy(buffer, this->_buffer + this->_front,
				this->_max_size - this->_front);
			memcpy(buffer + this->_max_size - this->_front,
				this->_buffer, this->_rear);
		}
		delete[] this->_buffer;
		this->_buffer = buffer;
		this->_front = 0;
		this->_rear = this->_max_size - 1;
		this->_max_size *= multiple;
	}
}

// End of file
