/* $Id: intelligence_i.hpp,v 1.16 2007-11-15 17:41:45 neo Exp $

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
/** @file intelligence_i.hpp
 *  @brief
 */

#ifndef __INTELLIGENCE_I_HPP__
#define __INTELLIGENCE_I_HPP__

#include "intelligence_x.hpp"

namespace checkers
{
	inline intelligence::intelligence(const board& board) :
		_board(board)
	{
	}

	// ================================================================

	/**
	 *  @retval >0 when the current player is ahead in game
	 *  @retval <0 when the current player is behind in game
	 */
	inline int intelligence::evaluate(void)
	{
		return this->evaluate_pieces() * evaluate::WEIGHT_PIECES +
			this->evaluate_kings() * evaluate::WEIGHT_KINGS +
			this->evaluate_movers() * evaluate::WEIGHT_MOVERS +
			this->evaluate_kings_row() *
			evaluate::WEIGHT_KINGS_ROW +
			this->evaluate_edges() * evaluate::WEIGHT_EDGES;
	}

	inline int intelligence::evaluate_pieces(void)
	{
		return this->_board.is_black_on_move() ?
			(this->_board.get_black_pieces().bitcount() -
			 this->_board.get_white_pieces().bitcount()) :
			(this->_board.get_white_pieces().bitcount() -
			 this->_board.get_black_pieces().bitcount());
	}

	inline int intelligence::evaluate_kings(void)
	{
		return this->_board.is_black_on_move() ?
			 (this->_board.get_black_kings().bitcount() -
			  this->_board.get_white_kings().bitcount()) :
			 (this->_board.get_white_kings().bitcount() -
			  this->_board.get_black_kings().bitcount());
	}

	inline int intelligence::evaluate_movers(void)
	{
		return this->_board.is_black_on_move() ?
			(this->_board.get_black_movers().bitcount() -
			 this->_board.get_white_movers().bitcount()) :
			(this->_board.get_white_movers().bitcount() -
			 this->_board.get_black_movers().bitcount());
	}

	inline int intelligence::evaluate_kings_row(void)
	{
		return this->_board.is_black_on_move() ?
			((this->_board.get_black_pieces() &
				bitboard::WHITE_KINGS_ROW).bitcount() -
			 (this->_board.get_white_pieces() &
				bitboard::BLACK_KINGS_ROW).bitcount()) :
			((this->_board.get_white_pieces() &
				bitboard::BLACK_KINGS_ROW).bitcount() -
			 (this->_board.get_black_pieces() &
				bitboard::WHITE_KINGS_ROW).bitcount());
	}

	inline int intelligence::evaluate_edges(void)
	{
		return this->_board.is_black_on_move() ?
			((this->_board.get_black_pieces() &
				bitboard::EDGES).bitcount() -
			 (this->_board.get_white_pieces() &
				bitboard::EDGES).bitcount()) :
			((this->_board.get_white_pieces() &
				bitboard::EDGES).bitcount() -
			 (this->_board.get_black_pieces() &
				bitboard::EDGES).bitcount());
	}

	inline void intelligence::reorder_moves(std::vector<move>& moves,
		unsigned int ply)
	{
		if (!this->_reorder)
		{
			return;
		}
		if (ply >= this->_best_moves.size())
		{
			this->_reorder = false;
			return;
		}

		std::vector<move>::iterator pos = std::find(moves.begin(),
			moves.end(), this->_best_moves[ply]);
		if (moves.end() == pos)
		{
			this->_reorder = false;
			return;
		}

		std::swap(moves[0], *pos);
	}

	inline void intelligence::set_timeout(time_t second)
	{
		intelligence::_deadline = timeval::now() + second;
	}

	inline bool intelligence::is_timeout(void)
	{
		return timeval::now() > intelligence::_deadline;
	}
}

#endif // __INTELLIGENCE_I_HPP__
// End of file
