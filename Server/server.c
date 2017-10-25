#include "headers.h"
#include "var.h"

int main(){
	int port;
	pthread_t thread1, thread2;
	int  iret1, iret2;
	printf("[Server]: Please enter the server port number:\n");
	scanf("%d",&port);
	/* Creating socket */
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(server_socket < 0){
		printf("Error in creating socket!!!!\n");
		return 0;
	}

	/* Configure settings of the server address struct */
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(port);
	ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	/* End */

	memset(ServerAddress.sin_zero, '\0', sizeof(ServerAddress.sin_zero));  

	/* Binding the address struct to the socket */
	if(bind(server_socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) < 0)
	{
		printf("Error in binding!!!!!\n");
		return 0;
	}

	/* Listen on the socket, with 5 max connection requests queued */
	if(listen(server_socket,5)==0)
		printf("Listening\n");
	else
		printf("Error\n");

	addr_size = sizeof(serverStorage);
	printf("Waiting for clients\n");
	Client_Socket = accept(server_socket, (struct sockaddr *) &serverStorage, &addr_size);
	printf("Please enter your message to client \n");

	if((iret1 = pthread_create( &thread1, NULL, Recv_From_Client, NULL))!=0)
	{
		printf("Thread creation failed: %d\n", iret1);

	}
	
	if((iret2 = pthread_create( &thread2, NULL, Send_To_Client, NULL))!=0)
	{
		printf("Thread creation failed: %d\n", iret2);

	}

	printf("pthread_create() for thread 1 returns: %d\n",iret1);
	printf("pthread_create() for thread 2 returns: %d\n",iret2);

	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL); 
	close(server_socket);
	close(Client_Socket);


	return 0;
}

