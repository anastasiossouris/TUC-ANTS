/*
 * tucants_all.hpp
 *
 *  Created on: May 16, 2013
 *      Author: croatoan
 */

#ifndef TUCANTS_ALL_HPP_
#define TUCANTS_ALL_HPP_

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//#define _DEBUG_		//comment this line when you are done

#ifdef _DEBUG_
    #include <assert.h>
#else
    #define assert(p) {}
#endif


#ifndef getOtherSide
	#define getOtherSide( a ) ( 1-(a) )
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/**********************************************************/
#define MAXIMUM_MOVE_SIZE 6

#define INITIAL_ANTS 12

/* The size of our board */
#define BOARD_COLUMNS 8
#define BOARD_ROWS 12
#define BOARD_SIZE 8

/* Values for each possible tile state */
#define WHITE 0
#define BLACK 1
#define EMPTY 2
#define RTILE 3
#define ILLEGAL 4

// max size of our name
#define MAX_NAME_LENGTH 16

//default port for client and server
#define DEFAULT_PORT "6001"


struct Move
{
	char tile[ 2 ][ MAXIMUM_MOVE_SIZE ];
	char color;
};

/* Position struct to store board, score and player's turn */
struct Position
{
	char board[ BOARD_ROWS ][ BOARD_COLUMNS ];
	char score[ 2 ];
	char turn;
};


/**********************************************************/
void initPosition( Position * pos );
//initializes position

void printBoard( char board[ BOARD_ROWS][ BOARD_COLUMNS ] );
//prints board

void printPosition( Position * pos );
// Prints board along with Player's turn and score

void doMove( Position * pos, Move * moveToDo );
//plays moveToDo on position pos
//Caution!!! does not check if it is legal! Simply does the move!

int canJump( char row, char col, char player, Position * pos );
// returns 1 if we can jump to the left 2 if we can jump to the right 3 if we can jump both directions and 0 if no jump is possible
// row,col can be empty. So it can also be used to determine if we can make a jump from a position we do not occupy
//Caution!!! does no checks if we are inside the board

int canJumpTo( char row, char col, char player, Position * pos, char rowDest, char colDest );
// like canJump() it doesn't need row, col to be occupied by a piece.
// Caution!!! does no checks if we are inside board

int canMove( Position * pos, char player );
// determines if player can move

int isLegal( Position * pos, Move * moveToCheck );
// determines if a move is leagal

/**********************************************************/
#define MAXPENDING 10
/**********************************************************/
#define NM_NEW_POSITION 1
#define NM_COLOR_W 2
#define NM_COLOR_B 3
#define NM_REQUEST_MOVE 4
#define NM_PREPARE_TO_RECEIVE_MOVE 5
#define NM_REQUEST_NAME 6
#define NM_QUIT 7
/**********************************************************/
extern char * port;
/**********************************************************/

void listenToSocket( char * port, int * mySocket );
//creates a socket and starts to listen (used by server)

int acceptConnection( int mySocket );
//accepts new connections (used by server)

void connectToTarget( char * port, char * ip, int * mySocket );
//connects to a server (used by client)

int sendMsg( int msg, int mySocket );
//sends a network message (one char)

int recvMsg( int mySocket );
//receives a network message

int sendMove( Move * moveToSend, int mySocket );
//sends a move via mySocket

int getMove( Move * moveToGet, int mySocket );
//receives a move from mySocket

void sendName( char textToSend[ MAX_NAME_LENGTH + 1 ], int mySocket );
//used to send agent's name to server

int getName( char textToGet[ MAX_NAME_LENGTH + 1 ], int mySocket );
//used to receive agent's name

int sendPosition( Position * posToSend, int mySocket );
//used to send position struct

void getPosition( Position * posToGet, int mySocket );
//used to receive position struct

#endif /* TUCANTS_ALL_HPP_ */
