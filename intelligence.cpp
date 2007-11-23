/* $Id: intelligence.cpp,v 1.28 2007-11-23 15:18:09 neo Exp $

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
/** @file intelligence.cpp
 *  @brief Artificial intelligence, alpha-beta pruning.
 */

#include <iomanip>
#include "intelligence.hpp"

namespace checkers
{
	int intelligence::alpha_beta_search(io& io,
		std::vector<move>& best_moves, unsigned int depth, int alpha,
		int beta, unsigned int ply)
	{
		if (0 == this->_nodes % 2 ^ 16)
		{
			io << io::flush;
			if (this->is_timeout() || io.lines_to_read() ||
				io.eof())
			{
	 			/// @retval TIMEOUT when timeout
				return intelligence::timeout();
			}
		}
		++this->_nodes;

		if (this->_board.is_winning())
		{
			return intelligence::win() - ply;
		}
		else if (this->_board.is_losing())
		{
			return -intelligence::win() + ply;
		}
		else if (0 >= depth)
		{
			best_moves.clear();
			return evaluate();
		}

		// Generate legal moves
		std::vector<move> legal_moves = this->_board.generate_moves();
		// Re-order the legal moves
		this->reorder_moves(legal_moves, ply);

		int val;
		std::vector<move> moves;

		for (std::vector<move>::const_iterator pos =
			legal_moves.begin(); pos != legal_moves.end(); ++pos)
		{
			// When capture piece in the last ply, search deeper
			if (1 == depth && pos->get_capture())
			{
				++depth;
			}

			intelligence intelligence(*this);
			val = intelligence._board.make_move(*pos) ?
				 intelligence.alpha_beta_search(io, moves,
					depth,     alpha,   beta, ply + 1) :
				-intelligence.alpha_beta_search(io, moves,
					depth - 1, -beta, -alpha, ply + 1);

			if (intelligence::timeout() == val)
			{
				return val;
			}
			if (val >= beta)
			{
				return beta;
			}
			if (val > alpha)
			{
				alpha = val;
				best_moves.clear();
				best_moves.push_back(*pos);
				best_moves.insert(best_moves.end(),
					moves.begin(), moves.end());
			}
		}

		return alpha;
	}

	/** @return Timeout or not.
	 */ 
	bool intelligence::think(io& io, std::vector<move>& best_moves,
		const board& board, unsigned int depth_limit, time_t time_limit,
		verbose show_detail)
	{
		unsigned int i;
		unsigned int depth;
		int val = 0;
		struct timeval start;
		struct timeval end;

		intelligence::set_timeout(time_limit);

		for (i = 0, depth = std::max(best_moves.size(),
			static_cast<std::vector<move>::size_type>(1U)), val = 0;
			depth <= depth_limit && val != intelligence::timeout();
			++i, ++depth)
		{
			intelligence::_nodes = 0;
			intelligence::_best_moves = best_moves;
			intelligence::_reorder = true;

			intelligence intelligence(board);
			start = timeval::now();
			val = intelligence.alpha_beta_search(io, best_moves,
				depth);
			end = timeval::now();

			if (intelligence::VERBOSE == show_detail)
			{
				intelligence::show_think(io, depth, val,
					end - start, intelligence::_nodes,
					best_moves, !(i % 8));
			}

			if (best_moves.size() < depth)
			{
				break;
			}
		}

		/** @retval true while timeout.
		 *  @retval false while reach specified search depth or game
		 *   end.
		 */
		return val == intelligence::timeout();
	}

	// ================================================================

	void intelligence::show_think(io& io, unsigned int depth, int val,
		struct timeval time, long unsigned int nodes,
		const std::vector<move>& best_moves, bool show_title)
	{
		std::ostringstream stream;

		if (show_title)
		{
			stream << "  depth   value      time       nodes\n";
			stream << "  ----------------------------------------"
				"------------------------------------\n";
		}
		stream << "  " << std::setw(5) << depth;
		stream << "  ";
		if (intelligence::timeout() == val)
		{
			stream << "     -";
		}
		else
		{
			stream << std::setw(6) << val;
		}
		stream << ' ' << std::setw(5) << time.tv_sec << '.' <<
			std::setw(3) << std::setfill('0') <<
			(time.tv_usec / 1000) << std::setfill(' ');
		stream << ' ' << std::setw(11) << nodes;
		stream << ' ';

		std::vector<move>::size_type i;
		std::vector<move>::size_type max_size = best_moves.size();
		for (i = 0; i < max_size; ++i)
		{
			if (i > 0 && 0 == i % 8)
			{
				stream << "\n                                "
					"      ";
			}
			stream << ' ' << best_moves[i];
		}
		stream << '\n';

		io << stream.str() << io::flush;
	}

	std::vector<move> intelligence::_best_moves;
	bool intelligence::_reorder = false;
	long unsigned int intelligence::_nodes = 0;
	struct timeval intelligence::_deadline = { 0, 0 };
}

// End of file
