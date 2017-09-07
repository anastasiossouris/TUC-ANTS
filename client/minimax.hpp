/*
 * minimax.hpp
 *
 *  Created on: May 6, 2013
 *      Author: croatoan
 */

#ifndef MINIMAX_HPP_
#define MINIMAX_HPP_

#include<stdexcept>
#include<list>
#include<utility>
#include<tuple>
#include<stack>
#include<limits>
#include"time_limit_cutoff_test.hpp"

namespace search{

/**
 * Game represents a game for the adversial search. The following elements must be specified
 * for the search to run.
 *
 * State, Action : the types of the actions made and the type of the states they lead to.
 * Each state object must have a function node_type() that returns whether the state is a min_node, a max_node
 * or a chance_node. These are enum values defined in this file.
 * UtilityType : type of the utility value
 * SuccessorsFunction : a functor object that must provide the following signature with the purpose
 * of returning all the actions, as well as the states they lead to, from the input state.
 * 					std::list<std::tuple<Action,State,double> > operator()(const State&)
 * where the third item in the tuple is the probability for that action in case that the input State is a chance node.
 * EvaluationFunction: a functor object that must provide the following signature:
 * 					utility_type operator()(const State&)
 * CutoffTest : the cutoff test for the search. This can be a non-pure functor object (that is the copy
 * received at the constructor of the search is preserved). The signature to be provided is:
 * 			bool operator()(const State&)
 * ActionOrdering : whose aim is to order the actions returned from the successorsfunction. It must
 * provide the following signature: void operator()(std::list<std::tuple<Action,State,double> >&)
 */
template<class Game>
struct game_traits{
	typedef typename Game::state_type state_type;
	typedef typename Game::action_type action_type;
	typedef typename Game::utility_type utility_type;
	typedef typename Game::successors_function_type successors_function_type;
	typedef typename Game::evaluation_function_type evaluation_function_type;
	typedef typename Game::cutoff_test_type cutoff_test_type;
	typedef typename Game::action_ordering_type action_ordering_type;

	// Returns the minus infinity for the range of values representable by the utility type
	static utility_type min_utility_value(){
		return Game::min_utility_value();
	}

	// Returns the plus infinity for the range of values representable by the utility type
	static utility_type max_utility_value(){
		return Game::max_utility_value();
	}

	// Returns the maximum of the two input parameters
	static utility_type max_utility_cmp(utility_type a, utility_type b){
		return Game::max_utility_cmp(a,b);
	}

	// Returns the minimum of the two input parameters
	static utility_type min_utility_cmp(utility_type a, utility_type b){
		return Game::min_utility_cmp(a,b);
	}

	// Returns less than zero, zero, or greater than zero if a if less than, equal to, or greater than b
	// accordingly.
	static int utility_cmp(utility_type a, utility_type b){
		return Game::utility_cmp(a, b);
	}

	// Returns whether the utility values are bounded. If they are (the first value in the tuple is true)
	// then the next two values in the tuple are the lowest value achievable and the highest value achievable
	// respectively. This is used to apply alpha-beta pruning for the chance nodes.
	static std::tuple<bool, utility_type, utility_type> bounded(){
		return Game::bounded();
	}
};

// For the expectiminimax algorithm each node can be one of three types:
// Max Node, Min Node or Chance Node
enum class StateNodeType  {MAX_NODE, MIN_NODE, CHANCE_NODE};

// The class that implements the expectiminimax algorithm with alpha-beta pruning.
// Specifically, it supports the following:
// 		1) Chance nodes
//		2) AB-Pruning
//		3) Action Ordering
//		4) Cutoff Test
//		5) Evaluation Function
//		6) Iterative Deepening with Timeout Cutoff
template<class Game>
class iterative_deepening_alpha_beta_expectiminimax{
public:
	typedef game_traits<Game> gtraits;

	typedef typename gtraits::state_type state_type;
	typedef typename gtraits::action_type action_type;
	typedef typename gtraits::utility_type utility_type;
	typedef typename gtraits::successors_function_type successors_function_type;
	typedef typename gtraits::evaluation_function_type evaluation_function_type;
	typedef typename gtraits::cutoff_test_type cutoff_test_type;
	typedef typename gtraits::action_ordering_type action_ordering_type;

	// constructor
	iterative_deepening_alpha_beta_expectiminimax(const cutoff_test_type& _cutoff = cutoff_test_type()) : cutoff(_cutoff){}

	// It returns the action to take as a result of the expectiminimax algorithm on the input state
	action_type decision(const state_type& state, unsigned int msec){
		// we apply iterative deepening with increasing values of depth.
		// When timeout expires, we return the move selected from the deepest search that has been completed
		timeout_cutoff timeout(msec);
		std::stack<action_type> actions;

		int max_depth = std::numeric_limits<int>::max();

		for (int depth = 0; depth < max_depth; ++depth){
			action_type action = decision_up_to_depth(state, depth, timeout);

			// if the timeout has expired
			if (timeout()){
				// then we choose the action selected from the deepest search that has been completed
				// that is from the action at the top of the stack unless the stack is empty
				return actions.empty() ? action : actions.top();
			}
			actions.push(action);
		}

		assert(!actions.empty());

		return actions.top();
	}

	// It returns the action to take as a result of the expectiminimax algorithm on the input state
	// with a limit for the depth parameter and a timeout cutoff test
	action_type decision_up_to_depth(const state_type& state, int depth, timeout_cutoff& timeout){
		/**
		 * We must choose the action maximizing the values we receive from each state we get after applying
		 * each valid action on the current state.
		 * So first get the actions and the states for the current state using successors()
		 * and then we apply the action ordering optimization.
		 */
		std::list<std::tuple<action_type,state_type,double> > actions = std::move(successors(state));
		action_order(actions);

		typedef typename std::list<std::tuple<action_type,state_type,double> >::iterator iterator;


		utility_type utility = gtraits::min_utility_value();
		iterator result = actions.begin();

		// For each next state
		for (iterator first = actions.begin(), last = actions.end(); first != last; ++first){
			utility_type current_utility = exp_minimax_value(std::get<1>(*first), gtraits::min_utility_value(), gtraits::max_utility_value(), depth, timeout);

			// If the utility of the current state is better (max) then
			// mark it in the result iterator.
			if (current_utility > utility){
				utility = current_utility;
				result = first;
			}
		}

		// At the end the result iterator points to the tuple that had the state with the maximum
		// value
		return std::get<0>(*result);
	}
private:
	cutoff_test_type cutoff;
	evaluation_function_type eval;
	successors_function_type successors;
	action_ordering_type action_order;

	// This is a dispatch method that according to the type of the state node (max node, min node, chance node)
	// it calls the appropriate function to calculate the value.
	utility_type exp_minimax_value(const state_type& state, utility_type a, utility_type b, int depth, timeout_cutoff& timeout){
		// First we apply uniformly to all state node types the cutoff optimization test.
		// We also stop if the timeout expires or the depth limit has been reached
		if (depth == 0 || cutoff(state) || timeout()){
			return eval(state);
		}

		switch(state.node_type()){
		case StateNodeType::MAX_NODE:
			return max_node_exp_minimax_value(state, a, b, depth, timeout);
			//break;
		case StateNodeType::MIN_NODE:
			return min_node_exp_minimax_value(state, a, b, depth, timeout);
			//break;
		case StateNodeType::CHANCE_NODE:
			return chance_node_exp_minimax_value(state, a, b, depth, timeout);
			//break;
		default:
			throw std::logic_error("alpha_beta_expectiminimax: invalid node type in exp_minimax_value()");
			//break;
		}
	}

	// Calculates the value of a max node (with alpha-beta pruning)
	utility_type max_node_exp_minimax_value(const state_type& state, utility_type a, utility_type b, int depth, timeout_cutoff& timeout){
		// Get the next states from the current state
		std::list<std::tuple<action_type,state_type,double> > actions = std::move(successors(state));
		// Apply action ordering optimization
		action_order(actions);

		typedef typename std::list<std::tuple<action_type,state_type,double> >::iterator iterator;

		// For each next state
		for (iterator first = actions.begin(), last = actions.end(); first != last; ++first){
			// Get the utility of the current next state
			utility_type current_utility = exp_minimax_value(std::get<1>(*first), a, b, depth - 1, timeout);

			// Apply alpha-beta pruning optimization
			a = gtraits::max_utility_cmp(a, current_utility);

			if (gtraits::utility_cmp(a, b) >= 0){
				return b;
			}
		}

		return a;
	}

	// Calculates the value of a min node (with alpha-beta pruning)
	utility_type min_node_exp_minimax_value(const state_type& state, utility_type a, utility_type b, int depth, timeout_cutoff& timeout){
		// Get the next states from the current state
		std::list<std::tuple<action_type,state_type,double> > actions = std::move(successors(state));
		// Apply action ordering optimization
		action_order(actions);

		typedef typename std::list<std::tuple<action_type,state_type,double> >::iterator iterator;

		utility_type utility = gtraits::max_utility_value();

		// For each next state
		for (iterator first = actions.begin(), last = actions.end(); first != last; ++first){
			// Get the utility of the current next state
			utility_type current_utility = exp_minimax_value(std::get<1>(*first), a, b, depth - 1, timeout);

			// Apply alpha-beta pruning optimization
			b = gtraits::min_utility_cmp(b, current_utility);

			if (gtraits::utility_cmp(b, a) <= 0){
				return a;
			}
		}

		return b;
	}

	utility_type chance_node_exp_minimax_value(const state_type& state, utility_type a, utility_type b, int depth, timeout_cutoff& timeout){
		std::list<std::tuple<action_type,state_type,double> > actions = std::move(successors(state));
		action_order(actions);

		typedef typename std::list<std::tuple<action_type,state_type,double> >::iterator iterator;

		utility_type utility = 0;

		for (iterator first = actions.begin(), last = actions.end(); first != last; ++first){
			utility_type current_utility = exp_minimax_value(std::get<1>(*first), a, b, depth - 1, timeout);

			utility += current_utility*std::get<2>(*first);
		}

		return utility;
	}
};

} // namespace search


#endif /* MINIMAX_HPP_ */
