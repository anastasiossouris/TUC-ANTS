/*
 * time_limit_cutoff_test.hpp
 *
 *  Created on: May 11, 2013
 *      Author: croatoan
 */

#ifndef TIME_LIMIT_CUTOFF_TEST_HPP_
#define TIME_LIMIT_CUTOFF_TEST_HPP_

#include<iostream>
#include<chrono>


	using std::chrono::steady_clock;
	using std::chrono::seconds;

	/*
	 * timeout_cutoff is used to determine a timeout that is defined as the number of milliseconds that can pass
	 * starting from "now", where now is defined as the point where the constructor is called with the given
	 * number of arguments.
	 */
	class timeout_cutoff{
	public:
		typedef steady_clock::time_point time_point;

		explicit timeout_cutoff(unsigned int milliseconds) : limit(steady_clock::now() + std::chrono::milliseconds(milliseconds)), passed(false), num_milliseconds(milliseconds){}

		// this little utility is non-copyable
		timeout_cutoff(const timeout_cutoff&) = delete;
		timeout_cutoff& operator=(const timeout_cutoff&) = delete;

		bool operator()() const{
			if (passed){
				return true;
			}
			return passed = (steady_clock::now() >= limit);
		}

		unsigned int milliseconds() const{
			return num_milliseconds;
		}
	private:
		time_point limit; // a time point indicating the time limit for the timeout
		mutable bool passed; // whether the limit is passed. initialized to false
					// it is set to true when the time limit is exhausted.
					// it is used to avoid making calls to time functions
		unsigned int num_milliseconds;
	};


#endif /* TIME_LIMIT_CUTOFF_TEST_HPP_ */
