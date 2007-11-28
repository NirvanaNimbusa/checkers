/* $Id: move.hpp,v 1.23 2007-11-28 17:17:21 neo Exp $

   This file is a part of ponder, a English/American checkers game.

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
/** @file move.hpp
 *  @brief Move.
 */

#ifndef __MOVE_HPP__
#define __MOVE_HPP__

#include <vector>
#include "bitboard.hpp"

namespace checkers
{
	class move
	{
	public:
		// Constructor
		inline move(bitboard src, bitboard dest, bitboard capture,
			bool will_capture_a_king, bool will_crown);

		/// Get the source square of the move.
		inline bitboard get_src(void) const;
		/// Get the destination square of the move.
		inline bitboard get_dest(void) const;
		/// Get the squareof the captured piece.
		inline bitboard get_capture(void) const;

		/// Whether this move will capture a king.
		inline bool will_capture_a_king(void) const;
		/// Whether this move will crown.
		inline bool will_crown(void) const;

		friend bool operator ==(const move& lhs, const move& rhs);

	private:
		/// The source square of the move.
		bitboard _src;
		/// The destination square of the move.
		bitboard _dest;
		/// The square of the captured piece.
		bitboard _capture;
		/// Whether this move will capture a king.
		bool _will_capture_a_king;
		/// Whether this move will crown.
		bool _will_crown;
	};

	/// Relational ``equal to''.
	inline bool operator ==(const move& lhs, const move& rhs);
	/// Relational ``not equal to''.
	inline bool operator !=(const move& lhs, const move& rhs);

	/// Stream out the move.
	std::ostream& operator <<(std::ostream& os, const move& rhs);
	/// Stream out the moves.
	std::ostream& operator <<(std::ostream& os,
		const std::vector<move>& rhs);
}

#include "move_i.hpp"
#endif // __MOVE_HPP__
// End of file
