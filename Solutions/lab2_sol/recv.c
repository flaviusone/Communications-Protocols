#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(void)
{
	msg t;
	my_pkt p;
	int filesize, read_so_far, fd;
	char filename[MSGSIZE];
	
	init(HOST, PORT);
	printf("[RECEIVER] Client rocks.\n");

	/* Wait for filename */	
	memset(t.payload, 0, sizeof(t.payload));
	if (recv_message(&t) < 0) {
		perror("[RECEIVER] Receive message");
		return -1;
	}
	
	p = *((my_pkt*) t.payload);
	if (p.type != TYPE1) {
		perror("[RECEIVER] Receive message");
		return -1;
	}
	
	/* Extract message type */
	printf("[RECEIVER] Message type: %d\n", p.type);
	/* Extract filename */
	memcpy(filename, p.payload, t.len - sizeof(int));
	sprintf(filename, "%s_recv", filename);//numele fisierului ce va fi creat
	printf("[RECEIVER] Filename: %s\n", filename);

	/* Send ACK for filename */	
	memset(t.payload, 0, sizeof(t.payload));
	memset(p.payload, 0, sizeof(p.payload));
	
	p.type = TYPE4;
	memcpy(p.payload, ACK_T1, strlen(ACK_T1));//ACK_T1 e un string
  	t.len = strlen(p.payload) + 1 + sizeof(int);//de ce +1 ?
	memcpy(t.payload, &p, t.len);
  	send_message(&t);

	/* Wait for filesize */
	memset(t.payload, 0, sizeof(t.payload));
	if (recv_message(&t) < 0) {
		perror("Receive message");
		return -1;
	}

	p = *((my_pkt*) t.payload);
	if (p.type != TYPE2) {
		perror("[RECEIVER] Receive message");
		return -1;
	}
	
	printf("[RECEIVER] Message type: %d\n", p.type);
	memcpy(&filesize, p.payload, sizeof(int));
	printf("[RECEIVER] Filesize: %d\n", filesize);

	/* Send ACK for filesize */
	memset(t.payload, 0, sizeof(t.payload));
	memset(p.payload, 0, sizeof(p.payload));
	
	p.type = TYPE4;
	memcpy(p.payload, ACK_T2, strlen(ACK_T2));
  	t.len = strlen(p.payload) + sizeof(int);
	memcpy(t.payload, &p, t.len);
  	send_message(&t);

	read_so_far = 0;
	fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

	/* Wait for file contents - TYPE 3 messages */
	printf("[RECEIVER] Gonna wait for file chunks.\n");
	while (read_so_far < filesize) {
		memset(t.payload, 0, sizeof(t.payload));
		
		if (recv_message(&t) < 0) {
			perror("[RECEIVER] Receive message");
			return -1;
		}
		
		p = *((my_pkt*) t.payload);
		if (p.type != TYPE3) {
			perror("[RECEIVER] Receive message");
			return -1;
		}

		read_so_far += t.len - sizeof(int); //adica count (nr bytes cititi in sender)
		write(fd, p.payload, t.len - sizeof(int));;
		
		memset(t.payload, 0, sizeof(t.payload));
		memset(p.payload, 0, sizeof(p.payload));
	
		p.type = TYPE4;
		memcpy(p.payload, ACK_T3, strlen(ACK_T3));
	  	t.len = strlen(p.payload) + sizeof(int);
		memcpy(t.payload, &p, t.len);
  		send_message(&t);
	}

	printf("[RECEIVER] All done.\n");
	close(fd);
		
  	return 0;
}
