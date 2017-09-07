/*
 * tucants_game.hpp
 *
 *  Created on: May 7, 2013
 *      Author: croatoan
 */

#ifndef TUCANTS_GAME_HPP_
#define TUCANTS_GAME_HPP_

/**
 * This header file contains all what is needed to run
 * expectiminimax for the tucants game (in essence it provides
 * the game traits mechanism).
 */

#include<cassert>
#include<cmath>
#include<algorithm>
#include<iterator>
#include<utility>
#include<tuple>
#include<list>
#include<limits>
#include"minimax.hpp"
#include"tucants_all.hpp"
#include"time_limit_cutoff_test.hpp"

// The state of the game.
struct tucants_game{
	Position pos; // the state of the player
	char player; // who color am i?

	bool is_chance_node;
	int food_obtained;
	Move move; // this one is only used for chance nodes. Which was the move that resulted in this chance node;

	int player_num_ants_captured; // how many ants have i captured
	int opponent_num_ants_captured; // how many ants the opponent has captured

	search::StateNodeType node_type() const{
		if (is_chance_node){
			return search::StateNodeType::CHANCE_NODE;
		}
		return pos.turn == player ? search::StateNodeType::MAX_NODE : search::StateNodeType::MIN_NODE;
	}

	void init(){
		is_chance_node = false;
		food_obtained = 0;
		player_num_ants_captured = 0;
		opponent_num_ants_captured = 0;
	}
};

static const int board_utilities[2][12][8] = {
		{ // from the white player point of view
				{0,2,0,2,0,2,0,2},
				{2,0,2,0,2,0,2,0},
				{0,2,0,2,0,2,0,2},
				{5,0,1,0,1,0,1,0},
				{0,1,0,1,0,1,0,5},
				{5,0,1,0,1,0,1,0},
				{0,1,0,1,0,1,0,5},
				{5,0,1,0,1,0,1,0},
				{0,1,0,1,0,1,0,5},
				{3,0,3,0,3,0,3,0},
				{0,3,0,3,0,3,0,3},
				{3,0,3,0,3,0,3,0}
		},
		{ // from the black player point of view
				{0,3,0,3,0,3,0,3},
				{3,0,3,0,3,0,3,0},
				{0,3,0,3,0,3,0,3},
				{5,0,1,0,1,0,1,0},
				{0,1,0,1,0,1,0,5},
				{5,0,1,0,1,0,1,0},
				{0,1,0,1,0,1,0,5},
				{5,0,1,0,1,0,1,0},
				{0,1,0,1,0,1,0,5},
				{2,0,2,0,2,0,2,0},
				{0,2,0,2,0,2,0,2},
				{2,0,2,0,2,0,2,0}
		}
};

// Returns true if the (i,j) position is inside the regions of the board and false otherwise.
inline bool is_inside_board(int i, int j){
	return (0 <= i) && (i < BOARD_ROWS) && (0 <= j) && (j < BOARD_COLUMNS);
}

// Returns whether at the positions (i1,j1) and (i2,j2) there are ants of the same color.
inline bool of_same_color(const Position& pos, int i1, int j1, int i2, int j2){
	// assume that there are ants there!
	return pos.board[i1][j1] == pos.board[i2][j2];
}

// Returns whether at position (i,j) at the given pos the cell is empty
inline bool is_empty_cell(const Position& pos, int i, int j){
	return pos.board[i][j] == 2;
}

inline bool has_ant(const Position& pos, int i, int j){
	return pos.board[i][j] <= 1;
}

// returns true if all the ants have been removed from the board
inline bool ants_all_removed(const Position& pos){
	for (int i = 0; i < BOARD_ROWS; ++i){
		for (int j = 0; j < BOARD_COLUMNS; ++j){
			if (has_ant(pos, i, j)){
				// an ant found!
				return false;
			}
		}
	}

	// no ant found at board
	return true;
}

inline bool is_starting_board(const tucants_game& game){
	const Position& pos = game.pos;

	// check if the white ants are in their starting positions
	if (pos.board[0][1] != WHITE){
		return false;
	}
	if (pos.board[0][3] != WHITE){
		return false;
	}
	if (pos.board[0][5] != WHITE){
		return false;
	}
	if (pos.board[0][7] != WHITE){
		return false;
	}

	if (pos.board[1][0] != WHITE){
		return false;
	}
	if (pos.board[1][2] != WHITE){
		return false;
	}
	if (pos.board[1][4] != WHITE){
		return false;
	}
	if (pos.board[1][6] != WHITE){
		return false;
	}

	if (pos.board[2][1] != WHITE){
		return false;
	}
	if (pos.board[2][3] != WHITE){
		return false;
	}
	if (pos.board[2][5] != WHITE){
		return false;
	}
	if (pos.board[2][7] != WHITE){
		return false;
	}

	// check if the black ants are in their starting positions
	if (pos.board[9][0] != BLACK){
		return false;
	}
	if (pos.board[9][2] != BLACK){
		return false;
	}
	if (pos.board[9][4] != BLACK){
		return false;
	}
	if (pos.board[9][6] != BLACK){
		return false;
	}

	if (pos.board[10][1] != BLACK){
		return false;
	}
	if (pos.board[10][3] != BLACK){
		return false;
	}
	if (pos.board[10][5] != BLACK){
		return false;
	}
	if (pos.board[10][7] != BLACK){
		return false;
	}

	if (pos.board[11][0] != BLACK){
		return false;
	}
	if (pos.board[11][2] != BLACK){
		return false;
	}
	if (pos.board[11][4] != BLACK){
		return false;
	}
	if (pos.board[11][6] != BLACK){
		return false;
	}

	return true;
}

// Returns number of ants that the move captures at the given position.
// That is, playing move at pos results at a number of ants being captured which is returned by the function.
inline int num_captured_ants(Move move){
	// if we aren't a captivity move then we return immediately
	if (!((move.tile[0][0] != -1) && ( abs( move.tile[ 0 ][ 0 ] - move.tile[ 0 ][ 1 ] ) == 2 ) && ( abs( move.tile[ 1 ][ 0 ] - move.tile[ 1 ][ 1 ] ) == 2 ))){
		return 0;
	}

	// Since we are a captivity move it can be easily observed that the number of captures we make
	// is how many jumps we made, which is equivalent to the position where -1 is minus 1.
	for (int i = 0; i < MAXIMUM_MOVE_SIZE; ++i){
		if (move.tile[0][i] == -1){
			return i - 1;
		}
	}

	// if we reach here all moves are captures
	return (5);
}

// Returns how many ants have been removed from the board represented by the given position
// for the player with the given color
inline int ants_removed(const Position& pos, char color){
	int present = 0;

	// num removed = initial ants - num present
	for (int i = 0; i < BOARD_ROWS; ++i){
		for (int j = 0; j < BOARD_COLUMNS; ++j){
			if (pos.board[i][j] == color){
				++present;
			}
		}
	}

	return INITIAL_ANTS - present;
}

// Return a pair containing the amount of food for each player. The first is for the white
// and the second for the black.
inline std::pair<int,int> food_amount(const tucants_game& game){
	return std::make_pair(game.pos.score[WHITE] - ants_removed(game.pos, WHITE) + (game.player == WHITE ? game.opponent_num_ants_captured : game.player_num_ants_captured), game.pos.score[BLACK] - ants_removed(game.pos, BLACK) + (game.player == BLACK ? game.opponent_num_ants_captured : game.player_num_ants_captured));
}

// For the (i,j) positions assuming that an ant is there of the given color return a pair of positions
// (where each position is a pair of i,j coordinates) of where that ant CAN go. Note this method doesn't
// take into account the board.
inline std::pair<std::pair<int,int>,std::pair<int,int> > move_once(int i, int j, char color){
	switch(color){
	case WHITE:
		return std::make_pair(std::make_pair(i+1, j-1), std::make_pair(i+1,j+1));
	case BLACK:
		return std::make_pair(std::make_pair(i-1,j-1), std::make_pair(i-1,j+1));
	default:
		assert(0 && "move_once() invalid color parameter");
		break;
	}
}

// The player with the given color has made a move from (i,j) to (x,y). This method makes one more
// move in the same direction.
inline std::pair<int,int> make_move_in_same_direction(int i, int j, int x, int y, char color){
	switch(color){
	case WHITE:
		// try to figure out the initial direction and move in the same way
		if (x==(i+1) && y == (j-1)){
			return std::make_pair(x+1,y-1);
		}
		else if (x==(i+1) && y == (j+1)){
			return std::make_pair(x+1, y+1);
		}
		else{
			assert(0 && "make_move_in_same_direction() invalid positions");
			return std::make_pair(x,y); // dummy return!
		}
	case BLACK:
		if (x==(i-1) && y == (j-1)){
			return std::make_pair(x-1,y-1);
		}
		else if (x==(i-1) && y == (j+1)){
			return std::make_pair(x-1, y+1);
		}
		else{
			assert(0 && "make_move_in_same_direction() invalid positions");
			return std::make_pair(x,y); // dummy return!
		}
	default:
		assert(0 && "make_move_in_same_direction() invalid color parameter");
		break;
	}
}

// continue a captivity move. Source is (i,j) and at (x,y) we found an opponent ant!
// we return a list of moves cause it may happen that after the first capture, we can do two more and thus
// we get one more move.
inline std::list<Move> which_moves_captivity_case(const Position& pos, int i, int j, int x, int y, char color){
	std::pair<int,int> next = make_move_in_same_direction(i, j, x, y, color);

	int next_x = next.first;
	int next_y = next.second;

	std::list<Move> moves;

	if (!is_inside_board(next_x, next_y)){
		return moves;
	}

	// if the cell at (next_x,next_y) isn't empty then we do nothing
	if (has_ant(pos, next_x,next_y)){
		return moves;
	}

	// now we can move there but we may be apply to do more captivity moves and we must check them also
	std::pair<std::pair<int,int>,std::pair<int,int> > possible = move_once(next_x, next_y, color);

	int x1 = possible.first.first;
	int y1 = possible.first.second;

	int x2 = possible.second.first;
	int y2 = possible.second.second;

	bool made_more_captures = false;

	if (is_inside_board(x1,y1) && has_ant(pos, x1, y1) && (pos.board[x1][y1] != color)){
		std::list<Move> new_moves = which_moves_captivity_case(pos, next_x, next_y, x1, y1, color);

		for (auto it = new_moves.begin(); it != new_moves.end(); ++it){
			Move m;

			m.color = color;

			m.tile[0][0] = i;
			m.tile[1][0] = j;
			m.tile[0][1] = next_x;
			m.tile[1][1] = next_y;

			for (int i = 2, j = 1; i < 6; ++i, ++j){
				m.tile[0][i] = it->tile[0][j];
				m.tile[1][i] = it->tile[1][j];

				if (m.tile[0][i] == -1){
					break;
				}
			}

			moves.push_back(m);

			made_more_captures = true;
		}
	}

	if (is_inside_board(x2,y2) && has_ant(pos, x2, y2) && (pos.board[x2][y2] != color)){
		std::list<Move> new_moves = which_moves_captivity_case(pos, next_x, next_y, x2, y2, color);

		for (auto it = new_moves.begin(); it != new_moves.end(); ++it){
			Move m;

			m.color = color;

			m.tile[0][0] = i;
			m.tile[1][0] = j;
			m.tile[0][1] = next_x;
			m.tile[1][1] = next_y;

			for (int i = 2, j = 1; i < 6; ++i, ++j){
				m.tile[0][i] = it->tile[0][j];
				m.tile[1][i] = it->tile[1][j];

				if (m.tile[0][i] == -1){
					break;
				}
			}

			moves.push_back(m);

			made_more_captures = true;
		}
	}

	if (!made_more_captures){
		assert(moves.empty() && "which_moves_captivity_case() : moves ought to be empty");

		Move move;

		move.color = color;

		move.tile[0][0] = i;
		move.tile[1][0] = j;
		move.tile[0][1] = next_x;
		move.tile[1][1] = next_y;
		move.tile[0][2] = -1;

		moves.push_back(move);
	}

	return moves;
}

// Returns the possible moves that an ant at the given (i,j) position can make.
// If no move can be made then an empty list is returned instead.
inline std::list<Move> which_moves(const Position& pos, int i, int j){
	assert(pos.board[i][j] <= 1 && "which_moves() : no ant at (i,j) position");

	char color = pos.board[i][j];

	// take the initial destinations
	std::pair<std::pair<int,int>, std::pair<int,int> > dest = move_once(i,j,color);

	std::list<Move> moves;

	// (x1,y1) is the first destination
	int x1 = dest.first.first;
	int y1 = dest.first.second;

	// (x2,y2) is the second destination
	int x2 = dest.second.first;
	int y2 = dest.second.second;

	// Cases for the first destination
	if (is_inside_board(x1, y1) && !of_same_color(pos, i, j, x1, y1)){
		// (x1,y1) can be either empty or have an opponent ant

		// if it is empty then we store this move only if the other move from (x2,y2) doesn't have
		// captivity precedence
		if (!has_ant(pos, x1, y1)){
			if (is_inside_board(x2, y2) && !of_same_color(pos, i, j, x2, y2) && has_ant(pos,x2,y2)){
				// do nothing if move cannot be made
				std::pair<int,int> next = make_move_in_same_direction(i, j, x2, y2, color);

				int next_x = next.first;
				int next_y = next.second;


				if (has_ant(pos, next_x,next_y)){
					Move move;

					move.color = color;

					move.tile[0][0] = i;
					move.tile[1][0] = j;
					move.tile[0][1] = x1;
					move.tile[1][1] = y1;
					move.tile[0][2] = -1;

					moves.push_back(move);
				}
			}
			else{
				Move move;

				move.color = color;

				move.tile[0][0] = i;
				move.tile[1][0] = j;
				move.tile[0][1] = x1;
				move.tile[1][1] = y1;
				move.tile[0][2] = -1;

				moves.push_back(move);
			}
		}
		else{
			// else there is an opponent ant there
			std::list<Move> capt_moves = which_moves_captivity_case(pos, i, j, x1, y1, color);

			moves.insert(moves.begin(), capt_moves.begin(), capt_moves.end());
		}
	}

	// Cases for the second destination
	if (is_inside_board(x2, y2) && !of_same_color(pos, i, j, x2, y2)){
		// (x2,y2) can be either empty or have an opponent ant

		// if it is empty then we store this move only if the other move from (x2,y2) doesn't have
		// captivity precedence
		if (!has_ant(pos, x2, y2)){
			if (is_inside_board(x1, y1) && !of_same_color(pos, i, j, x1, y1) && has_ant(pos,x1,y1)){
				// do nothing if move cannot be made
				std::pair<int,int> next = make_move_in_same_direction(i, j, x1, y1, color);

				int next_x = next.first;
				int next_y = next.second;

				// if the cell at (next_x,next_y) isn't empty then we do nothing
				if (has_ant(pos, next_x,next_y)){
					Move move;

					move.color = color;

					move.tile[0][0] = i;
					move.tile[1][0] = j;
					move.tile[0][1] = x2;
					move.tile[1][1] = y2;
					move.tile[0][2] = -1;

					moves.push_back(move);
				}
			}
			else{
				Move move;

				move.color = color;

				move.tile[0][0] = i;
				move.tile[1][0] = j;
				move.tile[0][1] = x2;
				move.tile[1][1] = y2;
				move.tile[0][2] = -1;

				moves.push_back(move);
			}
		}
		else{
			// else there is an opponent ant there
			std::list<Move> capt_moves = which_moves_captivity_case(pos, i, j, x2, y2, color);

			moves.insert(moves.begin(), capt_moves.begin(), capt_moves.end());
		}
	}

	return moves;
}

// This is a functor object that serves as the successor function object for the
// minimax algorithm. It works in the following way:
// It gets as input a Position. That position holds the board as well as who player has turn.
// For that player that has now turn it returns a list of actions the player can make and the state they
// lead to.
struct tucants_successor_function{
	std::list<std::tuple<Move,tucants_game,double> > operator()(const tucants_game& game) const{
		char turn = game.pos.turn;

		std::list<std::tuple<Move,tucants_game,double> > all_moves;

		// if we are at a chance node then we have different handling
		if (game.node_type() == search::StateNodeType::CHANCE_NODE){
			tucants_game g1 = game;
			tucants_game g2 = game;
			tucants_game g3 = game;

			Move move = game.move;

			/**
			 * First of all let us notice that we can have at most 2 food cells in a move (because food appears
			 * in the middle cells only).
			 * So we count the number of food cells first.
			 * See the report for what we do in each case.
			 */
			int num_food_cells = 0;
			for (int i = 0; i < MAXIMUM_MOVE_SIZE; ++i){
				if (move.tile[0][i] == -1){
					break;
				}

				if (game.pos.board[move.tile[0][i]][move.tile[1][i]] == RTILE){
					++num_food_cells;
				}
			}

			switch(num_food_cells){
			case 1:
				// none of them are chance nodes now

				// the first one with probability 1/3 obtains the food
				g1.is_chance_node = false;
				g1.food_obtained = 1;

				all_moves.push_back(std::make_tuple(g1.move, g1, static_cast<double>(1.0/3.0)));

				// the second one with probability 2/3 destroys the food
				g2.is_chance_node = false;
				g2.food_obtained = 0;

				all_moves.push_back(std::make_tuple(g2.move, g2, static_cast<double>(2.0/3.0)));
				break;
			case 2:
				// none of them are chance nodes now

				// the first one with probability 4/9 obtains no food
				g1.is_chance_node = false;
				g1.food_obtained = 0;

				all_moves.push_back(std::make_tuple(g1.move, g1, static_cast<double>(4.0/9.0)));

				// the second one with probability 4/9 obtains one food
				g2.is_chance_node = false;
				g2.food_obtained = 1;

				all_moves.push_back(std::make_tuple(g2.move, g2, static_cast<double>(4.0/9.0)));

				// the third one with probability 1/9 obtains 2 food
				g3.is_chance_node = false;
				g3.food_obtained = 2;

				all_moves.push_back(std::make_tuple(g3.move, g3, static_cast<double>(1.0/9.0)));
				break;
			default:
				assert(0 && "tucants_successor_function: chance node invalid food cells count");
				break;
			}

			return all_moves;
		}

		// we are not a chance node
		for (int i = 0; i < BOARD_ROWS; ++i){
			for (int j = 0; j < BOARD_COLUMNS; ++j){
				// for each of the ants in board for the player who has turn
				if (game.pos.board[i][j] == turn){
					// for that ant of the player get all the moves it can make
					std::list<Move> moves = std::move(which_moves(game.pos,i,j));

					// record each move as well as the state they lead to
					for (auto it = moves.begin(); it != moves.end(); ++it){
						// the new state for the current move
						tucants_game new_game = game;

						new_game.is_chance_node = false;
						new_game.food_obtained = 0;

						// we must now apply the move to that new state
						doMove(&new_game.pos, &(*it));

						all_moves.push_back(std::make_tuple(*it, new_game, static_cast<double>(0.0)));
					}
				}
			}
		}

		// here we look out for chance nodes. More specifically if we land to a cell with food we baptize
		// that node a chance node
		std::for_each(all_moves.begin(), all_moves.end(), [](std::tuple<Move,tucants_game,double>& elem){
			Move& move = std::get<0>(elem);
			tucants_game& game = std::get<1>(elem);

			// just search if we have a cell with food there
			for (int i = 0; i < MAXIMUM_MOVE_SIZE; ++i){
				if (move.tile[0][i] == -1){
					break;
				}

				if (game.pos.board[move.tile[0][i]][move.tile[1][i]] == RTILE){
					game.is_chance_node = true;
					game.food_obtained = 0;
					game.move = move;
					break;
				}
			}
		});

		// if we have moves that have captivity precedence then we must filter out the normal moves
		std::list<std::tuple<Move,tucants_game,double> > captivity_moves;

		std::copy_if(all_moves.begin(), all_moves.end(), std::back_inserter(captivity_moves), [](const std::tuple<Move,tucants_game,double>& elem) -> bool{
			// must check here whether the move is a captivity move
			Move move = std::get<0>(elem);

			return num_captured_ants(move) != 0;
		});

		if (!captivity_moves.empty()){
			return captivity_moves;
		}
		else{
			return all_moves;
		}
	}
};

// this is the cutoff test for the tucants game.
struct tucants_game_cutoff{
	// return true if we must cutoff
	bool operator()(const tucants_game& game) const{
		// cutoff when the game has ended
		return ants_all_removed(game.pos);
	}
};

// return the utility of the board from the player's side given
inline int player_utility(const tucants_game& game, char player){
	int value = 0;

	for (int i = 0; i < BOARD_ROWS; ++i){
		for (int j = 0; j < BOARD_COLUMNS; ++j){
			if (game.pos.board[i][j] == player){
				// give value to this ant according to its position in the board
				value += board_utilities[player][i][j];

				// give additional value if it can make moves and captivity moves
				std::list<Move> moves = std::move(which_moves(game.pos,i,j));

				int captivity_moves = 0;
				for (auto it = moves.begin(); it != moves.end(); ++it){
					captivity_moves += num_captured_ants(*it);
				}

				value += (moves.size() + captivity_moves);

				// give additional value if the ant can protect another ant
				std::pair<std::pair<int,int>,std::pair<int,int> > move = move_once(i, j, player);

				int x1 = move.first.first;
				int y1 = move.first.second;

				int x2 = move.second.first;
				int y2 = move.second.second;

				if (is_inside_board(x1, y1) && (game.pos.board[x1][y1] == player)){
					++value;
				}
				if (is_inside_board(x2, y2) && (game.pos.board[x2][y2] == player)){
					++value;
				}
			}
		}
	}

	// but now loose utility if the opponent can capture an ant from me
	char opponent = 1 - player;

	tucants_successor_function succ;
	tucants_game game2 = game;
	game2.pos.turn = opponent;

	std::list<std::tuple<Move,tucants_game,double> > moves = succ(game2);

	// count from those moves how many ants it can capture
	int num_captures = 0;
	for (auto it = moves.begin(); it != moves.end(); ++it){
		num_captures += num_captured_ants(std::get<0>(*it));
	}

	value -= (2*num_captures);

	// also give more value if at that state we have a food captured
	value += 2*(game.pos.turn == player ? game.food_obtained : 0);

	return value;
}

// this is the evaluation function for the tucants game
struct tucants_evaluation_function{
	int operator()(const tucants_game& game) const{
		return (player_utility(game, game.player) + game.pos.score[game.player]) - (player_utility(game, 1 - game.player) + game.pos.score[1 - game.player]);
	}
};

// this is the action ordering for the tucants game.
struct tucants_action_ordering{
	void operator()(std::list<std::tuple<Move,tucants_game,double> >& successors) const{
		tucants_evaluation_function eval;
		// sort in ascending order by evaluation function
		successors.sort([&](const std::tuple<Move,tucants_game,double>& a, const std::tuple<Move,tucants_game,double>& b) -> bool{
			// return true if a is less than b by the evaluation function
			return eval(std::get<1>(a)) < eval(std::get<1>(b));
		});
	}
};

// the game traits
struct tucants{
	typedef tucants_game state_type;
	typedef Move action_type;
	typedef int utility_type;
	typedef tucants_successor_function successors_function_type;
	typedef tucants_evaluation_function evaluation_function_type;
	typedef tucants_game_cutoff cutoff_test_type;
	typedef tucants_action_ordering action_ordering_type;

	// Returns the minus infinity for the range of values representable by the utility type
	static utility_type min_utility_value(){
		return std::numeric_limits<utility_type>::min();
	}

	// Returns the plus infinity for the range of values representable by the utility type
	static utility_type max_utility_value(){
		return std::numeric_limits<utility_type>::max();
	}

	// Returns the maximum of the two input parameters
	static utility_type max_utility_cmp(utility_type a, utility_type b){
		return std::max(a,b);
	}

	// Returns the minimum of the two input parameters
	static utility_type min_utility_cmp(utility_type a, utility_type b){
		return std::min(a,b);
	}

	// Returns less than zero, zero, or greater than zero if a if less than, equal to, or greater than b
	// accordingly.
	static int utility_cmp(utility_type a, utility_type b){
		if (a < b){
			return -1;
		}
		else if (a==b){
			return 0;
		}
		else{
			return 1;
		}
	}

	// Returns whether the utility values are bounded. If they are (the first value in the tuple is true)
	// then the next two values in the tuple are the lowest value achievable and the highest value achievable
	// respectively. This is used to apply alpha-beta pruning for the chance nodes.
	static std::tuple<bool, utility_type, utility_type> bounded(){
		return std::make_tuple(false,0,0);
	}
};

#endif /* TUCANTS_GAME_HPP_ */
