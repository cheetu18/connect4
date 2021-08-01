#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#define ROW 6
#define COL 7
#define EMPTY ' '
#define SERVER 0
#define CLIENT 1
char* game_arr[COL]; char* current_player; char* myport;
char token;
char player1[50]; //server 
char player2[50]; //client
int pos, turn_num, c, win, turn = 0;
char buf[2] = {'0', '\0'};

//function declarations 
char accept_input();
void whose_turn(int* i);
void display_world(char c);

//depending on wheter client or server, sock will be set to either client_sockfd or server_sockfd;
int sock;      
void teardown()
{
	//printf("\033[0m");
	printf("\nDestroying the game");
	for(int i = 0; i < COL; i++){
		free(game_arr[i]);
	}
}

/*error handling for accept_input function*/ 
void check_pos(int* c)
{
/*removes case sensitivity i.e 'A' and 'a' are the same */
	if(*c > 96)
		*c-=32;

	
/*'Q' & 'q' will quit game*/
	if(*c == 81){
	printf("Quitting the game");
		teardown();
		exit(1);
		   }
/*anything outside of A-G will ask user to try again*/
	if(*c < 65 || *c > 71){
		printf("Invalid character.Valid characters are between A-G.\n");
		accept_input();	
		 }	
/*try again if column is full */
	if(game_arr[5][*c-65] != EMPTY){
		printf("Cannot insert into full column.Please Try again!\n");	
		accept_input();
	}

}
//diagonal right win check
int wincheck_dr(int row, int col)
{
	if(col >= 3 || row < 3){
		return 0;
	}
	int i = 0;
	while(i < 3){
		if(game_arr[--row][++col]  != token){
			return 0;
		}
		i++;
	}
	return 1;
}
//diagonal left win check
int wincheck_dl(int row, int col)
{
	if(col < 3 || row < 3){
		return 0; 	
	}
	int i = 0;
	while(i < 3){
		if(game_arr[--row][--col] != token){
			return 0;
		}
		i++;
	}
	return 1;

}
//vertical win check
int wincheck_v(int row, int col)
{
	if(row >= 3){
		for(int i = row-1; i >= row-3; i--){
			if(game_arr[i][col]  != token){
				return 0;
			}
		}
			return 1;
	}
		return 0;
}	
//horizontal win check 
int wincheck_h(int row, int col)
{
int tnum = 0; 
	for(int i = 0; i < COL; i++){

		if(tnum == 4){
			return 1;
		}

		if(game_arr[row][i] != token){
			tnum = 0;
			}
		else{
			tnum++;
		}
	}
		return 0;		

}
char accept_input()
{
		
	printf("%s:Enter a location on the board: ", current_player);
	scanf(" %c", &c);
	check_pos(&c);
	return c;
}
		
int is_full(int pos){
for(int i = 0; i <= COL; i++){
	if(game_arr[5][i] == EMPTY){
		return 0;
	}
}
	return 1;
}
//check for win condition 
void update_world(char c)
{		
	int row;
	int pos = c - 65;


	if(is_full(pos) == 1){
	printf("Draw!\nGame Over!!!\n");
	teardown();
	exit(1);
	}

for(int i = 0; i <= COL; i++){
	if(game_arr[i][pos] == EMPTY){
	       row = i;
	       game_arr[i][pos] = token;	
	       break;
	}

}

if(wincheck_v(row, pos) || wincheck_dr(row, pos) || wincheck_dl(row, pos) || wincheck_h(row, pos)){
	win = 1;
}
}
//set pointer to current player's name 
void whose_turn(int* i)
{
	if(*i % 2 == 0){
   		current_player = player1;
		token = 'X';
		//printf("\033[0;35m");
	}
	else{
		current_player = player2;
		token = '0';
		//printf("\033[0;32m");
	}
}
//creating visual representation of board 
void display_world(char c)
{
	for(int i = ROW - 1; i >= 0; i--){
		for(int j = 0; j < COL; j++){
		
		printf(" |  %c ", game_arr[i][j]);

		}
		puts("  |\n");
	}		

		printf("Inserted disc into column: %c\n",c);
}
int main(int argc, char **argv)
{ 
	for(int i = 0; i < COL; i++){
		 game_arr[i] = malloc(ROW * sizeof(int));
	}
	
	for(int i = 0; i < ROW; i++){
		for(int j = 0; j < COL; j++)
			game_arr[i][j] = ' ';
			}
	//client code 
	if(argc == 3){
		turn = CLIENT;
		int status, client_sockfd;
		struct addrinfo hints, *res;
	
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		if((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0){
			fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));		
			exit(1);
		}	

		if((client_sockfd = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol)) == -1){
			fprintf(stderr, "socket creation error: %s\n", gai_strerror(client_sockfd));		
			exit(1);
		}

		
		if((connect(client_sockfd, res -> ai_addr, res -> ai_addrlen)) == -1){
			perror("cannot connect");		
			exit(1);
		}
		
			freeaddrinfo(res); //free res structure 

			//Start of Game: Client 
			recv(client_sockfd, player1, 25, 0);
			printf("Player 2, enter your name:\n");
			fgets(player2, sizeof(player2), stdin);
			send(client_sockfd, player2, 25, 0); 
			sock = client_sockfd;

		}
	//server network code
	else{
		 turn = SERVER;
		 int server_sockfd;
		 printf("Setting up the game\n");

		//server port 
		if(argc == 2)
			myport = argv[1];
		else
			myport = "12345";
		
		int status, sock_fd;
		int opt = 1;
		struct addrinfo  hints, *res;
		struct sockaddr_storage their_addr;
		socklen_t addr_size;
		memset(&hints, 0, sizeof(hints)); //make sure hints is empty
		hints.ai_family = AF_UNSPEC; //ipv6 or ipv4 doesn't matter
		hints.ai_socktype = SOCK_STREAM; //require TCP stream sockets
		hints.ai_flags = AI_PASSIVE;
		
		//load res adderinfo struct 
		if((status = getaddrinfo(NULL, myport, &hints, &res)) != 0){
			fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));		
			exit(1);
			}
		
		
		//create socket with res
		if((sock_fd = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol)) == -1){
			fprintf(stderr, "socket creation error: %s\n", gai_strerror(sock_fd));		
			exit(1);
			}

		//reuseable socket
		if(setsockopt(sock_fd,SOL_SOCKET, SO_REUSEADDR,	&opt, sizeof(opt)) == -1){
			perror("setsockopt");
			exit(1);
			}
		//bind socket for use 
		if((bind(sock_fd, res -> ai_addr, res-> ai_addrlen)) == -1){
			perror("binding fail");
			exit(1);
			}
		
		
		//listen with a backlog of 5 
		listen(sock_fd, 5);

		addr_size = sizeof(their_addr);
		if((server_sockfd = accept(sock_fd, (struct sockaddr *)&their_addr, &addr_size)) == -1){
			perror("accepting failure");
			exit(1);
			}

		freeaddrinfo(res); //free res structure 
		
		//Start of Game: Server 
		printf("Player 1, please enter your name:\n"); 	
		fgets(player1, sizeof(player1), stdin);
		send(server_sockfd, player1, 25, 0); 
		recv(server_sockfd, player2, 25, 0);
		sock = server_sockfd;   
	}
	
	while(win == 0){

		printf("The turn number is: %i\n", turn_num);
		whose_turn(&turn_num);
		printf("Turn:%s", current_player);

		
	if(turn_num % 2 == turn){ 
		//even is server  odd is client  
		accept_input(&c);
		check_pos(&c);
		update_world(c);
		display_world(c);
		buf[0] = c;
		//printf("THis is your char: %c\n\n", buf[0]);
		send(sock, buf, strlen(buf), 0);  	 	
		turn_num ++;
		if(win == 1){
			printf("You win!\n");
			printf("Game Over!!!");
		}
		}
	else{
		printf("waiting for partner to move...\n");
		recv(sock, buf,strlen(buf), 0);	
		c = buf[0];
		update_world(c);
		display_world(c);
		turn_num ++;
		if(win == 1){
			printf("You lose!\n");
			printf("Game over!!!");
			}
		}

	}
	teardown();
	return 0;
}
