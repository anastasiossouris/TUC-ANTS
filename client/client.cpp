#include"tucants_all.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include"tucants_game.hpp"

// timeout in milliseconds
#define TIMEOUT 1000

/**********************************************************/
//Position gamePosition;		// Position we are going to use
tucants_game gamePosition;

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket

char * agentName = "croatoan";		

char * ip = "127.0.0.1";	// default ip (local machine)
/**********************************************************/

void print_game_information(const tucants_game& game){
	std::cout << std::endl;

	std::cout << "I have captured: " << game.player_num_ants_captured << " ants" << std::endl;
	std::cout << "My opponent has captured: " << game.opponent_num_ants_captured << " ants" << std::endl;

	std::pair<int,int> food = food_amount(game);

	std::cout << "White has captured: " << food.first << " amount of food" << std::endl;
	std::cout << "Black has captured: " << food.second << " amount of food" << std::endl;
}

#if 1
int main( int argc, char ** argv )
{
	int c;
	extern char *optarg;
   	 extern int optind, optopt, opterr;
	opterr = 0;
	unsigned int timeout = TIMEOUT;
	const char* timeout_string = 0;

	while( ( c = getopt ( argc, argv, "i:p:t:a:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port]\n" );
				return 0;
			case 'i':
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case 't':
				timeout_string = optarg;
				break;
			case 'a':
				agentName = optarg;
				break;
			case '?':
				if( optopt == 'i' || optopt == 'p' )
					printf( "Option -%c requires an argument.\n", ( char ) optopt );
				else if( isprint( optopt ) )
					printf( "Unknown option -%c\n", ( char ) optopt );
				else
					printf( "Unknown option character -%c\n", ( char ) optopt );
				return 1;
			default:
			return 1;
		}


	// if a timeout string has been given as command line then we get the string as the number for the timeout
	if (timeout_string != 0){
		std::string val(timeout_string);

		timeout = std::stoi(val);
	}

	connectToTarget( port, ip, &mySocket );

	char msg;

/**********************************************************/
// used in random
	srand( time( NULL ) );
	int i, j, k;
	int jumpPossible;
	int playerDirection;
/**********************************************************/

	gamePosition.init();

	int previous_ants_removed_num = 0;

	while( 1 )
	{
		print_game_information(gamePosition);

		msg = recvMsg( mySocket );

		switch ( msg )
		{
			case NM_REQUEST_NAME:		//server asks for our name
				sendName( agentName, mySocket );
				break;

			case NM_NEW_POSITION:		//server is trying to send us a new position
				std::cout << "Hello guys!" << std::endl;
				getPosition( &gamePosition.pos, mySocket );
				// gamePosition now holds the new position of the game
				printPosition( &gamePosition.pos );

				if (is_starting_board(gamePosition)){
					std::cout << "Reset received. Starting anew!" << std::endl;
					gamePosition.init();
					previous_ants_removed_num = 0;
				}
				else if (gamePosition.pos.turn == myColor){
					// this is the board after the move the opponent has made
					// we must update the count of the number of ants the opponent has captured from me
					int next_ants_removed_num = ants_removed(gamePosition.pos, myColor);

					gamePosition.opponent_num_ants_captured += (next_ants_removed_num - previous_ants_removed_num);
					previous_ants_removed_num = next_ants_removed_num;
				}

				break;

			case NM_COLOR_W:			//server informs us that we have WHITE color
				myColor = WHITE;
				gamePosition.player = myColor;
				printf("My color is %d\n",myColor);
				break;

			case NM_COLOR_B:			//server informs us that we have BLACK color
				myColor = BLACK;
				gamePosition.player = myColor;
				printf("My color is %d\n",myColor);
				break;

			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;


				if( !canMove( &gamePosition.pos, myColor ) )
				{
					myMove.tile[ 0 ][ 0 ] = -1;		//null move
				}
				else
				{
					// here is where we run expectiminimax on the current position
					// and it is our move the algorithm returns which action to do
					tucants_game_cutoff cutoff;
					search::iterative_deepening_alpha_beta_expectiminimax<tucants> minimax(cutoff);

					myMove = minimax.decision(gamePosition, timeout);
				}

				// count how many we will capture
				gamePosition.player_num_ants_captured += num_captured_ants(myMove);
				assert(myMove.color == gamePosition.player);
				sendMove( &myMove, mySocket );			//send our move
				
				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
		}

	} 

	return 0;
}
#endif
