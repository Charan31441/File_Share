char buffer_server[1024];
struct sockaddr_in ServerAddress,serverStorage;
socklen_t addr_size;

int Client_Socket;
void status_bar(int i,int total){
	int j;
	int percent = (25*i)/total;
	printf("[");
	for(j=0;j<=percent;j++){
		printf(">");
	}
	for(j=0;j<25-percent;j++){
		printf(" ");
	}
	if((percent*4) == 100)
	printf("] %d %% \n",percent*4);
	else	
	printf("] %d %% \r",percent*4);

}

void recv_file_from_client(char FILENAME[]){
	ssize_t len;
	char buffer_client[BUFSIZ];
	int file_size;
	FILE *received_file;
	int remain_data = 0;

	/* Receiving file size */
	recv(Client_Socket, buffer_client, BUFSIZ, 0);
	file_size = atoi(buffer_client);
	//fprintf(stdout, "\nFile size : %d\n", file_size);

	received_file = fopen(FILENAME, "w");
	if (received_file == NULL)
	{
		fprintf(stderr, "Failed to open file %s --> %s\n", FILENAME,strerror(errno));
		return ;

	}

	remain_data = file_size;

	while (((remain_data > 0) && ((len = recv(Client_Socket, buffer_client, BUFSIZ, 0)) > 0))) 
	{
		//fwrite(buffer_client, sizeof(char), len, received_file);
		remain_data -= len;
		//fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", len, remain_data);
		status_bar(file_size - remain_data, file_size);
		//fflush(stdout);
	}
	printf("------------Completed Receiving File-------\n");
	fclose(received_file);
}

void upload_file_to_Client(char FILE_TO_SEND[]){
	int fd;
	int sent_bytes = 0;
	char file_size[BUFSIZ];
	struct stat file_stat;
	fd = open(FILE_TO_SEND, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "Error opening file --> %s", strerror(errno));

		return;
	}

	/* Get file stats */
	if (fstat(fd, &file_stat) < 0)
	{
		fprintf(stderr, "Error fstat --> %s", strerror(errno));

		return;
	}

	fprintf(stdout, "File Size: \n%d bytes\n", file_stat.st_size);

	sprintf(file_size, "%d", file_stat.st_size);

	/* Sending file size */
	int len = send(Client_Socket, file_size, sizeof(file_size), 0);
	if (len < 0)
	{
		fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));

		return;
	}

	fprintf(stdout, "Server sent %d bytes for the size\n", len);

	int offset = 0;
	int remain_data = file_stat.st_size;
	printf("remaining data: %d\n", remain_data);
	/* Sending file data */
	while (((sent_bytes = sendfile(Client_Socket, fd, offset, BUFSIZ)) > 0) && (remain_data >= 0))
	{
		//fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
		remain_data -= sent_bytes;
		//fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
		status_bar(file_stat.st_size - remain_data, file_stat.st_size);
		fflush(stdout);
	}
	close(fd);
	printf("----------Completed uploading----------\n");
}

void *Recv_From_Client(){
	while(1){
		bzero(buffer_server, sizeof(buffer_server));
		if(recv(Client_Socket, buffer_server, 1024, 0) > 0){
			printf("[Client]>>: %s\n", buffer_server);
			int n = strlen(buffer_server);
			char FILE_TO_SEND[256];
			if( n > 4){
				if((buffer_server[n-4]==' ' && buffer_server[n-3]=='U' && buffer_server[n-2]=='D' && buffer_server[n-1]=='P') || (buffer_server[n-4]==' ' && buffer_server[n-3]=='T' && buffer_server[n-2]=='C' && buffer_server[n-1]=='P')){
					printf("Recv the file...\n");
					char FILENAME[BUFSIZ];
					strncpy(FILENAME, buffer_server,n-4);
					printf("%s\n", FILENAME);
					FILE_TO_SEND[n-3] = '\0';
					recv_file_from_client(FILENAME);
				}
			}
		}
		else{
			return 0;
		}
	}
}
void *Send_To_Client(){
	while(1){
		bzero(buffer_server, sizeof(buffer_server));
		getchar(); 
		//printf("[Server]>>: ");
		scanf("%[^\n]s",buffer_server);
		if(send(Client_Socket,buffer_server,sizeof(buffer_server),0) < 0){
			return 0;
		}
		int n = strlen(buffer_server);
		char FILE_TO_SEND[256];
		if( n > 4){
			if((buffer_server[n-4]==' ' && buffer_server[n-3]=='U' && buffer_server[n-2]=='D' && buffer_server[n-1]=='P') || (buffer_server[n-4]==' ' && buffer_server[n-3]=='T' && buffer_server[n-2]=='C' && buffer_server[n-1]=='P')){
				strncpy(FILE_TO_SEND, buffer_server,n-4);
				FILE_TO_SEND[n-3] = '\0';
				printf("The upload file name is %s\n", FILE_TO_SEND);
				upload_file_to_Client(FILE_TO_SEND);
			}
		}
	}
}
