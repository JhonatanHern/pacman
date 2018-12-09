#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int GHOST_WAIT = 750000,gameEnded = 0,level = 1;

struct position{
	int x;
	int y;
};

char **map = NULL;

int maxX,
	maxY,
	ghostID = 1,
	dots = 0;

struct position player,ghosts[5];

sem_t mapWatcher,numberAssign;
void displayMap(){
	char charToPrint;
	printw("Level %d\n",level);
	for (int y = 0; y < maxY; ++y){
		for (int x = 0; x < maxX; ++x){
			if (y==player.y && x == player.x){
				charToPrint = 'P';
			}else if(y==ghosts[1].y && x == ghosts[1].x){
				charToPrint = '1';
			}else if(y==ghosts[2].y && x == ghosts[2].x){
				charToPrint = '2';
			}else if(y==ghosts[3].y && x == ghosts[3].x){
				charToPrint = '3';
			}else if(y==ghosts[4].y && x == ghosts[4].x){
				charToPrint = '4';
			}else{
				charToPrint = map[y][x];
			}
			printw("%c",charToPrint);
		}
		printw("\n");
	}
}
void updateMap(){
	map[player.y][player.x] = ' ';
}
void youLost(){
	clear();
	printw("You Lost! :(\nPress q to end");
	refresh();
	while(getchar()!='q');//loop until the user presses q
	endwin();
	exit(0);
}
void youWin(){
	char input = 'k';
	clear();
	printw("You Won! :)\nPress n to next level\nPress q to end");
	refresh();
	do{
		input = getchar();
	}while( input != 'n' && input != 'q' );
	if (input=='n'){
		gameEnded = 1;//true
		sem_post(&mapWatcher);
		pthread_exit(NULL);
	}else{
		endwin();
		exit(0);
	}
}
int ghostCanMove(int x, int y, char mov,int id){
	char next;
	switch(mov){
		case 'a'://left
			for (int i = 0; i < 4; ++i){
				if ( i != id && ghosts[i].x == x-1  && ghosts[i].y == y ){
					return 0;
				}
			}
			next = map[y][x-1];
			break;
		case 'd'://right
			for (int i = 0; i < 4; ++i){
				if ( i != id && ghosts[i].x == x+1  && ghosts[i].y == y ){
					return 0;
				}
			}
			next = map[y][x+1];
			break;
		case 'w'://up
			for (int i = 0; i < 4; ++i){
				if ( i != id && ghosts[i].x == x  && ghosts[i].y == y-1 ){
					return 0;
				}
			}
			next = map[y-1][x];
			break;
		case 's'://down
			for (int i = 0; i < 4; ++i){
				if ( i != id && ghosts[i].x == x  && ghosts[i].y == y+1 ){
					return 0;
				}
			}
			next = map[y+1][x];
			break;
	}
	switch (next){
		case ' ':
		case '.':
		case 'P':
			return 1;
	}
	return 0;
}
void ghostMove(int id,char direction){
	switch(direction){
		case 'a':
			ghosts[id].x--;
			break;
		case 's':
			ghosts[id].y++;
			break;
		case 'd':
			ghosts[id].x++;
			break;
		case 'w':
			ghosts[id].y--;
			break;
	}
}
int userMove(int x, int y, char mov){
	char next;
	switch(mov){
		case 'a'://left
			next = map[y][x-1];
			break;
		case 'd'://right
			next = map[y][x+1];
			break;
		case 'w'://up
			next = map[y-1][x];
			break;
		case 's'://down
			next = map[y+1][x];
			break;
		default:
			return 0;
	}
	switch(next){
		case '.':
			dots--;
			if (dots==0){
				youWin();
				return 1;
			}
		case '1':
		case '2':
		case '3':
		case '4':
		case ' ':
			switch(mov){
				case 'a'://left
					player.x--;
					break;
				case 'd'://right
					player.x++;
					break;
				case 'w'://up
					player.y--;
					break;
				case 's'://down
					player.y++;
					break;
			}
			return 1;
	}
	return 0;
}
int colision(){
	for (int i = 1; i < 5; ++i){
		if (ghosts[i].x == player.x && ghosts[i].y == player.y){
			return 1;
		}
	}
	return 0;
}
char antiDirection(char c){
	switch(c){
		case 'a':
			return 'd';
		case 'w':
			return 's';
		case 's':
			return 'w';
		case 'd':
			return 'a';
	}
	return 'f';
}
char *directions = "awsd";
char getRandomDirection(){
	return directions[random() % 4];
}
/* 
 * User:
 * 1 - define variables
 * 2 - wait 4 char
 * 3 - if invalid char go to (2)
 * 4 - if chr == 'o' exit program
 * 5 - if invalid move go to (2)  
 * 6 - move (semaphore for map handle)
 * 7 - verify colision w ghost (if colision die)
 * 8 - remove dot
 * 9 - dotcounter--
 * 10 - if dotcounter==0 end. Victory. 
*/

int validChar(char c){
	switch(c){
		case 'a':
		case 'w':
		case 's':
		case 'd':
			return true;
	}
	return false;
}
void *userAction(void *param){
	int ch;
	displayMap();
	ch = getch();
	while(ch != 113){
		if (validChar(ch)){
			sem_wait(&mapWatcher);
			if (userMove( player.x , player.y , ch )){
				if (colision()){
					sem_post(&mapWatcher);
					youLost();
				}
	 			clear();
				updateMap();
				displayMap();
		 		refresh();
			}
			sem_post(&mapWatcher);
		}
	 	ch = getch();
	}
	endwin();
	exit(0);
}

int absolute(int n){
	return n >= 0 ? n : - n ;
}

char getPersecutionDirection(int ghostID){
	if(absolute(ghosts[ghostID].x - player.x) >  absolute(ghosts[ghostID].y - player.y) ){
		return ghosts[ghostID].x > player.x ? 'a' : 'd' ;//horizontal move
	}else{
		return ghosts[ghostID].y > player.y ? 'w' : 's' ;//vertical move
	}
}
/*
 * 1 - define variables & get ghost id
 * 2 - get persecution direction
 * 3 - if valid direction && direction != antiDirection( lastDirection ) : go to 6
 * 4 - get random direction
 * 5 - if !valid direction || direction == antiDirection( lastDirection ) : return to step 4
 * 6 - move
 * 7 - lastDirection = direction
 * 8 - refresh screen
 * 9 - if killedUser : u lost
 * 9 - sleep
 * 10 - go to 2)
*/
int counter = 0;
void *ghostAction(void *param){
	int id,iterations;//my ghost's id
	char direction,lastDirection = 'x';
	refresh();
	sem_wait(&numberAssign);
	id = ghostID;
	ghostID++;//avoid 2 ghosts w the same address
	sem_post(&numberAssign);
	while(1){
		sem_wait(&mapWatcher);
		direction = getPersecutionDirection(id);
		iterations = 0;
		if ( ! ghostCanMove( ghosts[id].x , ghosts[id].y , direction , id ) || direction == antiDirection(lastDirection)){
			do{
				direction = getRandomDirection();
				iterations++;
			}while(
				(
					! ghostCanMove( ghosts[id].x , ghosts[id].y , direction , id ) ||
					direction == antiDirection(lastDirection)
				)
				&&
				iterations < 20
				);
		}
		if (iterations<20){
			ghostMove(id,direction);
			lastDirection = direction;
			clear();
			displayMap();
			refresh();
			if ( ghosts[id].x == player.x && ghosts[id].y == player.y ) {
				youLost();
			}
		}else{
			lastDirection = antiDirection(lastDirection);
		}	
		sem_post(&mapWatcher);
		if (gameEnded){
			pthread_exit(NULL);
		}
		usleep(GHOST_WAIT);
	}
}

/*
 * Even when the map is a critical resource, the next function won't 
 * use the semaphores because is called only when no tread but the
 * main one is running
*/
void readMap(char const *path){
	int chr=0;
	FILE *file = fopen(path,"r");
	
	if ( map == NULL ){
		map = malloc(sizeof (char*) * maxY);
		for (int i = 0; i < maxY; ++i){
			map[i] = malloc(sizeof(char) * maxX);
		}
	}

	for (int y = 0; y < maxY; ++y){
		for (int x = 0; x < maxX + 1; ++x){
			chr = (char) fgetc(file);
			if (x < maxX){
				map[y][x] = chr;
				switch((char)chr){
					case '*':
						break;
					case '.':
						dots++;
						break;
					case '1':
					case '2':
					case '3':
					case '4':
						ghosts[ chr - 48 ].x = x;
						ghosts[ chr - 48 ].y = y;
						map[y][x] = ' ';
						break;
					case 'P':
						player.x = x;
						player.y = y;
						map[y][x] = ' ';
						break;
				}
			}
		}
	}
	fclose(file);
}
void debugMessage(const char *message){
	clear();
	printw(message);
	refresh();
	getchar();
}

int main(int argc, char const *argv[]){
	pthread_t user,ghostThreads[4];

	sem_init(&mapWatcher, 0, 1); 
	sem_init(&numberAssign, 0, 1); 
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	
	maxY = atoi( argv[1] );
	maxX = atoi( argv[2] );

	while(1){
		readMap( argv[3] );
		ghostID = 1;
		clear();
		pthread_create(&user,NULL,userAction,NULL);
		for (int i = 0; i < 4; ++i){
			refresh();
			pthread_create(&ghostThreads[i],NULL,ghostAction,NULL);
			usleep(250000);
		}

		pthread_join(user,NULL);
		for (int i = 0; i < 4; ++i){
			pthread_join(ghostThreads[i],NULL);
		}
		gameEnded = 0;
		GHOST_WAIT *= 0.75;
		level++;
	}

	endwin();
	return 0;
}