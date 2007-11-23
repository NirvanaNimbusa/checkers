/* $Id: engine.cpp,v 1.37 2007-11-23 15:18:09 neo Exp $

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
/** @file engine.cpp
 *  @brief Game engine.
 */

#include "engine.hpp"
#include "intelligence.hpp"

namespace checkers
{
	engine::engine(void) :
		_board(), _rotate(false), _history(), _best_moves(),
		_force_mode(false), _depth_limit(UNLIMITED), _time_limit(10),
		_io(STDIN_FILENO, STDOUT_FILENO)
	{
		this->_action.insert(std::make_pair("?",
			&engine::do_help));
		this->_action.insert(std::make_pair("analyze",
			&engine::do_analyze));
		this->_action.insert(std::make_pair("black",
			&engine::do_black));
		this->_action.insert(std::make_pair("force",
			&engine::do_force));
		this->_action.insert(std::make_pair("go",
			&engine::do_go));
		this->_action.insert(std::make_pair("help",
			&engine::do_help));
		this->_action.insert(std::make_pair("history",
			&engine::do_history));
		this->_action.insert(std::make_pair("new",
			&engine::do_new));
		this->_action.insert(std::make_pair("ping",
			&engine::do_ping));
		this->_action.insert(std::make_pair("ponder",
			&engine::not_implemented));
		this->_action.insert(std::make_pair("print",
			&engine::do_print));
		this->_action.insert(std::make_pair("quit",
			&engine::do_quit));
		this->_action.insert(std::make_pair("rotate",
			&engine::do_rotate));
		this->_action.insert(std::make_pair("sd",
			&engine::do_sd));
		this->_action.insert(std::make_pair("st",
			&engine::do_st));
		this->_action.insert(std::make_pair("setboard",
			&engine::do_setboard));
		this->_action.insert(std::make_pair("undo",
			&engine::do_undo));
		this->_action.insert(std::make_pair("white",
			&engine::do_white));
	}

	engine& engine::init(void)
	{
		static engine engine;

		return engine;
	}

	void engine::run(void)
	{
		std::string command;
		std::vector<std::string> args;
		std::map<std::string, do_action>::const_iterator pos;

		this->_board.opening();
		this->print_board();

		for (;;)
		{
			this->prompt();
			this->ponder();

			if (this->_io.eof())
			{
				break;
			}

			this->_io >> command;

			args = engine::parse(command);
			if (!args.empty())
			{
				pos = this->_action.find(args[0]);
				if (pos != this->_action.end())
				{
					(this->*pos->second)(args);
				}
				else if (!this->human_makes_move(args[0]))
				{
					this->computer_makes_move();
				}
			}
		}
	}

	// ================================================================

	void engine::print_board(void)
	{
		int i;
		int j;

		if (this->_rotate)
		{
			this->_io <<
				"       H   G   F   E   D   C   B   A\n"
				"     +---+---+---+---+---+---+---+---+\n";
			for (i = 1; i <= 8; ++i)
			{
				this->_io << "  " << i;
				if (i % 2)
				{
					this->_io << "  | ";
				}
				for (j = i * 4 - 1; j >= i * 4 - 4; --j)
				{
					this->_io << "  |";
					this->print_square(j);
					this->_io << "| ";
				}
				if (!(i % 2))
				{
					this->_io << "  | ";
				}
				this->_io << ' ' << i << "\n"
				"     +---+---+---+---+---+---+---+---+\n";
			}
			this->_io <<
				"       H   G   F   E   D   C   B   A\n";
		}
		else
		{
			this->_io <<
				"       A   B   C   D   E   F   G   H\n"
				"     +---+---+---+---+---+---+---+---+\n";
			for (i = 8; i >= 1; --i)
			{
				this->_io << "  " << i;
				if (!(i % 2))
				{
					this->_io << "  | ";
				}
				for (j = i * 4 - 4; j <= i * 4 - 1; ++j)
				{
					this->_io << "  |";
					this->print_square(j);
					this->_io << "| ";
				}
				if (i % 2)
				{
					this->_io << "  | ";
				}
				this->_io << ' ' << i << "\n"
				"     +---+---+---+---+---+---+---+---+\n";
			}
			this->_io <<
				"       A   B   C   D   E   F   G   H\n";
		}
	}

	void engine::print_square(int n)
	{
		assert(0 <= n && n < 32);

		bitboard square(0x1U << n);

		if (this->_board.get_black_men() & square)
		{
			this->_io << "(b)";
		}		
		else if (this->_board.get_white_men() & square)
		{
			this->_io << "(w)";
		}		
		else if (this->_board.get_black_kings() & square)
		{
			this->_io << "(B)";
		}		
		else if (this->_board.get_white_kings() & square)
		{
			this->_io << "(W)";
		}
		else
		{
			this->_io << " \\ ";
		}
	}

	std::string engine::to_string(int v)
	{
		if (UNLIMITED == v)
		{
			return std::string("unlimited");
		}
		std::ostringstream stream;
		stream << v;
		return stream.str();
	}

	int engine::to_int(const std::string& str)
	{
		if ("unlimited" == str)
		{
			return UNLIMITED;
		}
		int v = ::strtol(str.c_str(), NULL, 10);
		if (v < 0 || v > UNLIMITED)
		{
			return UNLIMITED;
		}
		return v;
	}

	std::vector<std::string> engine::parse(const std::string& command)
	{
		std::vector<std::string> args;
		std::string::size_type idx_begin = 0;
		std::string::size_type idx_end   = 0;
		std::string piece;
		std::string::const_iterator begin = command.begin();

		while ((idx_end = command.find(' ', idx_begin))
			!= std::string::npos)
		{
			piece = std::string(begin + idx_begin,
				begin + idx_end);
			idx_begin = idx_end + 1;
			if (!piece.empty())
			{
				args.push_back(piece);
			}
		}

		if ((idx_end = command.find('\n', idx_begin))
			!= std::string::npos)
		{
			piece = std::string(begin + idx_begin,
				begin + idx_end);
		}
		else
		{
			piece = std::string(begin + idx_begin, command.end());
		}

		if (!piece.empty())
		{
			args.push_back(piece);
		}

		return args;
	}

	bool engine::make_move(const move& move)
	{
		this->_history.push_back(move);

		bool ret = this->_board.make_move(move);

		if (this->_best_moves.size() > 0)
		{
			if (move == this->_best_moves[0])
			{
				this->_best_moves.erase(
					this->_best_moves.begin());
			}
			else
			{
				this->_best_moves.clear();
			}
		}

		return ret;
	}

	void engine::computer_makes_move(void)
	{
		if (this->_force_mode)
		{
			return;
		}

		bool contin;

		this->_io << "  Thinking ...\n";

		do
		{
			intelligence::think(this->_io, this->_best_moves,
				this->_board, this->_depth_limit,
				this->_time_limit, intelligence::VERBOSE);
			if (this->_best_moves.empty())
			{
				break;
			}
			do
			{
				this->_io << "move " << this->_best_moves[0]
					<< '\n';
				contin = this->make_move(this->_best_moves[0]);
				this->print_board();
			} while (contin && !this->_best_moves.empty());
		} while (contin);
		this->result();
	}

	/// @return whether human player move one more
	bool engine::human_makes_move(const std::string& str)
	{
		bool contin = true;

		try
		{
			move move = this->_board.generate_move(str);
			assert(this->_board.is_valid_move(move));
			contin = this->make_move(move);

			this->print_board();
			this->result();
		}
		catch (const std::logic_error& e)
		{
			this->_io << e.what() << '\n';
		}

		return contin;
	}

	void engine::idle(void)
	{
		for (;;)
		{
			this->_io << io::flush;
			if (this->_io.lines_to_read() || this->_io.eof())
			{
				break;
			}
			usleep(500);
		}
	}

	void engine::ponder(void)
	{
		if (this->_force_mode || !intelligence::think(this->_io,
			this->_best_moves, this->_board, engine::UNLIMITED,
			engine::UNLIMITED, intelligence::SILENT))
		{
			this->idle();
		}
	}

	void engine::prompt(void)
	{
		this->_io << "  *** "
			<< (this->_board.is_black_to_move() ? "Black" : "White")
			<< " move ***\n";
	}

	bool engine::result(void)
	{
		if (this->_board.is_winning())
		{
			this->_io << "RESULT "
				<< (this->_board.is_black_to_move() ?
				"1-0 {Black win}\n" :
				"0-1 {White win}\n");
			return true;
		}
		else if (this->_board.is_losing())
		{
			this->_io << "RESULT "
				<< (this->_board.is_black_to_move() ?
				"0-1 {White win}\n" :
				"1-0 {Black win}\n");
			return true;
		}

		return false;
	}

	void engine::do_analyze(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->_io << "  Analyzing ...\n";
		intelligence::think(this->_io, this->_best_moves, this->_board,
			this->_depth_limit, this->_time_limit,
			intelligence::VERBOSE);
	}

	void engine::do_print(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->print_board();
	}

	void engine::do_rotate(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->_rotate = !this->_rotate;
		this->print_board();
	}

	void engine::do_black(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->_board.set_black_to_move();
		this->_best_moves.clear();
	}

	void engine::do_white(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->_board.set_white_to_move();
		this->_best_moves.clear();
	}

	void engine::do_ping(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->_io << "pong";
		if (1 < args.size())
		{
			this->_io << ' ' << args[1];
		}
		this->_io << '\n';
	}

	void engine::do_go(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->_force_mode = false;
		this->computer_makes_move();
	}

	void engine::do_help(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->_io <<
			"    analyze         Engine thinks about what move it make next if it were on\n"
			"                    move.\n"
			"    black           Set Black on move, and the engine will play White.\n"
			"    force           Set the engine to play neither color (\"force mode\").\n"
			"    go              Leave force mode and set the engine to play the color that\n"
			"                    is on move.  Start thinking and eventually make a move.\n"
			"    help            Show this help information.\n"
			"    history         Show the record of moves.\n"
			"    new             Reset the board to the standard starting position.\n"
			"    ping N          N is a decimal number.  Reply by sending the string\n"
			"                    \"pong N\"\n"
			"    print           Show the current board.\n"
			"    quit            Quit this program.\n"
			"    rotate          Rotate the board 180 degrees.\n"
			"    setboard FEN    Set up the pieces position on the board.\n"
			"    sd DEPTH        The engine should limit its thinking to DEPTH ply.\n"
			"    st TIME         Set the time control to TIME seconds per move.\n"
			"    white           Set White on move, and the engine will play Black.\n"
			"    undo            Back up a move.\n";
	}

	void engine::do_history(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		board board;
		board.opening();
		for (std::vector<move>::const_iterator pos =
			this->_history.begin(); pos != this->_history.end();
			++pos)
		{
			this->_io << "  " <<
				(board.is_black_to_move() ? "Black" : "White")
				<< " " << (pos->get_capture() ? "jump" : "move")
				<< " " << *pos << '\n';
			board.make_move(*pos);
		}
	}

	void engine::do_new(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->_board.opening();
		this->_history.clear();
		this->_best_moves.clear();
		this->print_board();
	}

	void engine::do_quit(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		exit(0);
	}

	void engine::do_force(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		this->_force_mode = true;
	}

	void engine::do_sd(const std::vector<std::string>& args)
	{
		if (args.size() <= 1)
		{
			this->_io << "Error (option missing): sd\n";
			return;
		}
		this->_depth_limit = this->to_int(args[1]);
	}

	void engine::do_st(const std::vector<std::string>& args)
	{
		if (args.size() <= 1)
		{
			this->_io << "Error (option missing): st\n";
			return;
		}
		this->_time_limit  = this->to_int(args[1]);
	}

	void engine::do_setboard(const std::vector<std::string>& args)
	{
		if (args.size() <= 1)
		{
			this->_io << "Error (option missing): setboard\n";
			return;
		}
		this->_board = board(args.size() > 2 ?
			(args[1] + ' ' + args[2]) : args[1]);
		this->_history.clear();
		this->_best_moves.clear();
		this->print_board();
		this->_io << io::flush;
	}

	void engine::do_undo(const std::vector<std::string>& args)
	{
		// Void the warning: unused parameter ‘args’
		(void)args;

		if (this->_history.size() > 0)
		{
			this->_board.undo_move(this->_history.back());
			this->_history.pop_back();
			this->_best_moves.clear();
			this->print_board();
		}
		else
		{
			this->_io << "Error (no move to undo): undo\n";
		}
	}

	void engine::not_implemented(const std::vector<std::string>& args)
	{
		this->_io << "Error (command not implemented): " << args[0] << '\n';
	}
}

// End of file
