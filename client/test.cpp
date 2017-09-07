/*
 * test.cpp
 *
 *  Created on: May 11, 2013
 *      Author: croatoan
 */
#include<algorithm>
#include<iostream>
#include<chrono>
#include<list>
#include<limits>
#include"time_limit_cutoff_test.hpp"
#include"tucants_all.hpp"
#include"tucants_game.hpp"
#include"minimax.hpp"
#include"array"


void test_land_cells(){
	for (int i = 0; i < 12; ++i){
		for (int j = 0; j < 8; ++ j){
			bool island = (i+j)%2;

			std::cout << island << " ";
		}
		std::cout << std::endl;
	}
}

Position pos;

void print_move(const Move& move){
	for (int i = 0; i < 6; ++i){
		std::cout << (int)move.tile[0][i]  << " ";
	}

	std::cout << std::endl;

	for (int i = 0; i < 6; ++i){
		std::cout << (int)move.tile[1][i]  << " ";
	}
}

#if 0
int main(){
	// Purpose:
	// Time minimax for increasing values of depth

	// First need to create a starting state to run our minimax on.
	// This is made randomly
	tucants_game gamePosition;
	gamePosition.player = BLACK;
	gamePosition.init();
	initPosition(&gamePosition.pos);

	// how far down at depth do we wish to reach at minimax?
	const int max_depth = 15;
	const int initial_depth = 1;

	// to keep measurements
	std::array<double, max_depth - initial_depth + 1> measurements;

	// for each depth get timing information
	for (int depth = initial_depth, i = 0; depth < max_depth; ++depth, ++i){
		auto start = std::chrono::steady_clock::now();

		tucants_game_cutoff cutoff;
		search::iterative_deepening_alpha_beta_expectiminimax<tucants> minimax(cutoff);

		// we do not a time limit so we implicitly specify a very large number of milliseconds as timeout
		timeout_cutoff timeout(std::numeric_limits<unsigned int>::max());

		minimax.decision_up_to_depth(gamePosition, depth, timeout);

		auto end =  std::chrono::steady_clock::now();
		auto diff = end - start;

		measurements[i] =  std::chrono::duration <double,  std::milli> (diff).count();

		std::cout << "Up to depth = " << depth << " need time: " << measurements[i] << " ms" << std::endl;
	}

	return (0);
}
#endif


