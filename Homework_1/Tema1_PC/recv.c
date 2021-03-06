/*
 * Author: Flavius Tirnacop
 * Date: 2 April 2012
 * File: recv.c
 * Description:Reciever file for the protocol implementation
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"
 static msg r, t;
 static int task_index;
 static char *filename;
 int task_0() 
 {
 	my_pkt p;					
	int res,read_so_far,f;		//f for file descriptor
								//res for error reporting
	int file_size;				//the file dimmension
	char *filename_out;			//final output filename

	printf("[RECEIVER] Receiver begins.\n");

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
	
	/* Extract filename */
	filename =(char *) malloc((t.len-sizeof(int)));
	filename_out =(char *) malloc((t.len-sizeof(int))+strlen("recv_"));
	sprintf(filename_out, "recv_%s", filename);
	printf("[RECEIVER] Filename: %s\n", filename_out);

	/* Send ACK for filename */	
	memset(t.payload, 0, sizeof(t.payload));
	memset(p.payload, 0, sizeof(p.payload));
	
	p.type = TYPE4;
	memcpy(p.payload, ACK_T1, strlen(ACK_T1));
	t.len = strlen(p.payload) + 1 + sizeof(int);
	memcpy(t.payload, &p, t.len);
	send_message(&t);
	
	/* Wait for file_size */
	memset(t.payload, 0, sizeof(t.payload));
	if (recv_message(&t) < 0) {
		perror("Receive message");
		return -1;
	}
	/* Extract file_size */
	p = *((my_pkt*) t.payload);
	if (p.type != TYPE2) {
		perror("[RECEIVER] Receive message");
		return -1;
	}

	memcpy(&file_size, p.payload, sizeof(int));
	printf("[RECEIVER] file_size: %d\n", file_size);

	/* Send ACK for file_size */
	memset(t.payload, 0, sizeof(t.payload));
	memset(p.payload, 0, sizeof(p.payload));

	p.type = TYPE4;
	memcpy(p.payload, ACK_T2, strlen(ACK_T2));
	t.len = strlen(p.payload) + sizeof(int);
	memcpy(t.payload, &p, t.len);
	send_message(&t);

	read_so_far = 0;
	f = open(filename_out, O_CREAT | O_WRONLY | O_TRUNC, 0644);

	while(read_so_far < file_size) {

		/* cleanup msg */
		memset(&r, 0, sizeof(msg));

		/* wait for message */
		res = recv_message(&r);		
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return -1;
		}

		p = *((my_pkt*) r.payload);
		if (p.type != TYPE3) {
			perror("[RECEIVER] Receive message");
			return -1;
		}

		/* Write to file */
		read_so_far += r.len - sizeof(int);
		write(f, p.payload, r.len - sizeof(int));;
		
		memset(r.payload, 0, sizeof(r.payload));
		memset(p.payload, 0, sizeof(p.payload));
		
		p.type = TYPE4;
		memcpy(p.payload, ACK_T3, strlen(ACK_T3));
		r.len = strlen(p.payload) + sizeof(int);
		memcpy(r.payload, &p, r.len);
		send_message(&r);
	}
	
	close(f);
	printf("[RECEIVER] All done.\n");

	return 0;

}
int task_1()
{
	my_pkt_t1 p;
	int seq_nr=0;
	p.seq_nr = seq_nr;
	int res,read_so_far,f,file_size;
 	char *filename_out;						//final output file_name

 	printf("[RECEIVER] Receiver begins.\n");

	/* Wait for filename until recieved */	
 	memset(t.payload, 0, sizeof(t.payload));
 	while(1) {
 		res = recv_message(&t);
 		if (res < 0) {
 			perror("[RECIEVER] Receive error. Exiting.\n");
 			return -1;
 		}
 		p = *((my_pkt_t1*) t.payload);
 		if (p.seq_nr == seq_nr)
 			break;
 	}
 	if (p.type != TYPE1) {
 		perror("[RECEIVER] Receive message");
 		return -1;
 	}

	/* Extract filename */
 	filename =(char *) malloc((t.len-sizeof(int)));
 	filename_out =(char *) malloc((t.len-sizeof(int))+strlen("recv_"));
 	memcpy(filename, p.payload, t.len - sizeof(int));
 	sprintf(filename_out, "recv_%s", filename);
 	printf("[RECEIVER] Filename: %s\n", filename_out);

	/* Send ACK for filename */	
 	memset(t.payload, 0, sizeof(t.payload));
 	memset(p.payload, 0, sizeof(p.payload));

 	p.type = TYPE4;
 	p.seq_nr = seq_nr;
 	memcpy(p.payload, ACK_T1, strlen(ACK_T1));
 	t.len = strlen(p.payload) + 1 + sizeof(int);
 	memcpy(t.payload, &p, t.len);
 	send_message(&t);

 	seq_nr++;

	/* Wait for file_size */
 	memset(t.payload, 0, sizeof(t.payload));
 	while(1) {
 		res = recv_message(&t);
 		p = *((my_pkt_t1*) t.payload);
 		if (p.seq_nr == seq_nr)
 			break;
 	}

 	if (p.type != TYPE2) {
 		perror("[RECEIVER] Receive message");
 		return -1;
 	}
 	memcpy(&file_size, p.payload, sizeof(int));
 	printf("[RECEIVER] file_size: %d\n", file_size);

	/* Send ACK for file_size */
 	memset(t.payload, 0, sizeof(t.payload));
 	memset(p.payload, 0, sizeof(p.payload));

 	p.seq_nr = seq_nr;
 	p.type = TYPE4;
 	memcpy(p.payload, ACK_T2, strlen(ACK_T2));
 	t.len = strlen(p.payload) + 2*sizeof(int);
 	memcpy(t.payload, &p,  sizeof(my_pkt_t1));
 	send_message(&t);

 	seq_nr++;
 	read_so_far = 0;
 	f = open(filename_out, O_CREAT | O_WRONLY | O_TRUNC, 0644);

 	printf("[RECEIVER] Computing packets.\n");
 	while (read_so_far < file_size) { 
 		memset(t.payload, 0, sizeof(t.payload));
		/* Wait until recieve expected seq_nr */
 		while(1) {
 			res = recv_message(&t);
 			p = *((my_pkt_t1*) t.payload);
 			if (p.seq_nr == seq_nr)
 				break;
 			if (p.type != TYPE3) {
 				perror("[RECEIVER] Receive message");
 				return -1;
 			}
 		}	

		/* Write to file */
 		read_so_far += t.len - 2 * sizeof(int);
 		write(f, p.payload, t.len - 2*sizeof(int));

		/* Prepare ACK frame */
 		memset(t.payload, 0, sizeof(t.payload));
 		memset(p.payload, 0, sizeof(p.payload));
 		p.type = TYPE4;
 		p.seq_nr = seq_nr;
 		memcpy(p.payload, ACK_T3, strlen(ACK_T3));
 		t.len = strlen(p.payload) + 2*sizeof(int);
 		memcpy(t.payload, &p, sizeof(my_pkt_t1));
 		send_message(&t);
 		seq_nr++;
 	}
 	close(f);
 	printf("[RECEIVER] All done.\n");
 	return 0;
}
int task_2()
{

 	int seq_nr=0;
 	my_pkt_t1 p;
 	p.seq_nr = seq_nr;
 	int i=0, res,read_so_far,f;
 	int file_size;
 	char *filename_out;
 	printf("[RECEIVER] Receiver begins.\n");

	/* Wait for filename */	
 	memset(t.payload, 0, sizeof(t.payload));
 	while(1) {
 		res = recv_message(&t);
 		if (res < 0){
			perror("[RECEIVER] Receive error. Exiting.\n");
			return -1;
		}
 		p = *((my_pkt_t1*) t.payload);
 		if (p.seq_nr == seq_nr)
 			break;
 	}
 	if (p.type != TYPE1) {
 		perror("[RECEIVER] Receive message");
 		return -1;
 	}

	/* Extract filename */
 	filename =(char *) malloc((t.len-sizeof(int)));
 	filename_out =(char *) malloc((t.len-sizeof(int))+strlen("recv_"));
 	memcpy(filename, p.payload, t.len - sizeof(int));
 	sprintf(filename_out, "recv_%s", filename);
 	printf("[RECEIVER] Filename: %s\n", filename_out);

	/* Send ACK for filename */	
 	memset(t.payload, 0, sizeof(t.payload));
 	memset(p.payload, 0, sizeof(p.payload));

 	p.type = TYPE4;
 	p.seq_nr = seq_nr;
 	memcpy(p.payload, ACK_T1, strlen(ACK_T1));
 	t.len = strlen(p.payload) + 1 + sizeof(int);
 	memcpy(t.payload, &p, t.len);
 	send_message(&t);

 	seq_nr++;

	/* Wait for file_size */
 	memset(t.payload, 0, sizeof(t.payload));
 	while(1) {
 		res = recv_message(&t);
 		p = *((my_pkt_t1*) t.payload);
 		if (p.seq_nr == seq_nr)
 			break;
 	}

 	if (p.type != TYPE2) {
 		perror("[RECEIVER] Receive message");
 		return -1;
 	}
 	memcpy(&file_size, p.payload, sizeof(int));
 	printf("[RECEIVER] file_size: %d\n", file_size);

	/* Send ACK for file_size */
 	memset(t.payload, 0, sizeof(t.payload));
 	memset(p.payload, 0, sizeof(p.payload));

 	p.seq_nr = seq_nr;
 	p.type = TYPE4;
 	memcpy(p.payload, ACK_T2, strlen(ACK_T2));
 	t.len = strlen(p.payload) + 2*sizeof(int);
 	memcpy(t.payload, &p,  sizeof(my_pkt_t1));
 	send_message(&t);

 	seq_nr++;

	/* Wait for window size */
 	memset(t.payload, 0, sizeof(t.payload));
 	while (1) {
 		res = recv_message(&t);
 		p = *((my_pkt_t1*) t.payload);
 		if (p.seq_nr == seq_nr)
 			break;
 	}
 	int window = 0;
 	memcpy(&window, &p.payload, sizeof(int));

	/* Send ACK for window size*/
 	memset(t.payload, 0, sizeof(t.payload));
 	memset(p.payload, 0, sizeof(p.payload));

 	p.seq_nr = seq_nr;
 	p.type = TYPE4;
 	memcpy(p.payload, ACK_T2, strlen(ACK_T2));
 	t.len = strlen(p.payload) + 2 * sizeof(int);
 	memcpy(t.payload, &p, sizeof(my_pkt_t1));
 	send_message(&t);
	printf("[RECIEVER] Window size = %d\n",window);
 	
 	/* Resetting seq_nr this time */
 	seq_nr=0;
 	read_so_far = 0;
 	f = open(filename_out, O_CREAT | O_WRONLY | O_TRUNC, 0644);

 	printf("[RECEIVER] Computing packets.\n");

 	while (read_so_far < file_size) { 

 		memset(&t.payload, 0, sizeof(t.payload));
 		int count = 0;
 		msg messages[window];
 		/* Fill up buffer of messages */
 		while (count < window && read_so_far<file_size) {
 			res = recv_message(&t);
 			p = *((my_pkt_t1*) t.payload);
 			count++;
 			messages[p.seq_nr % window] = t;
 			seq_nr = p.seq_nr;
 			if (p.type != TYPE3) {
 				perror("[RECEIVER] Receive message");
 				return -1 ;
 			}
 			read_so_far += t.len - 2 * sizeof(int); 

 			memset(t.payload, 0, sizeof(t.payload));
 			memset(p.payload, 0, sizeof(p.payload));

 			p.type = TYPE4;
 			p.seq_nr = seq_nr;
 			memcpy(p.payload, ACK_T3, strlen(ACK_T3));
 			t.len = strlen(p.payload) + 2 * sizeof(int);
 			memcpy(t.payload, &p, sizeof(my_pkt_t1));
 			send_message(&t);
 		}
 		/* Buffer is full , write it in file */
 		for (i = 0; i < count; i++) {
 			p = *((my_pkt_t1*) messages[i].payload);
 			write(f, p.payload, messages[i].len - 2 * sizeof(int));
 		}

 	}
 	close(f);

 	printf("[RECEIVER] All done.\n");

 	return 0;
}
 int task_3()
 {

 	if (recv_message(&r) < 0) {
 		perror("[RECEIVER] receive message");
 		return -1;
 	}

 	printf("[RECEIVER] Got msg with payload: %s\n", r.payload);

 	sprintf(t.payload, "ACK(%s)", r.payload);
 	t.len = strlen(t.payload + 1);
 	send_message(&t);

 	printf("[RECEIVER] All done.\n");

 	return 0;

 }
 int task_4()
 {

 	if (recv_message(&r) < 0) {
 		perror("[RECEIVER] receive message");
 		return -1;
 	}

 	printf("[RECEIVER] Got msg with payload: %s\n", r.payload);

 	sprintf(t.payload, "ACK(%s)", r.payload);
 	t.len = strlen(t.payload + 1);
 	send_message(&t);

 	printf("[RECEIVER] All done.\n");

 	return 0;

 }

 int main(int argc, char *argv[])
 {
 	task_index = atoi(argv[1]);	
 	printf("[RECEIVER] Receiver starts.\n");
 	printf("[RECEIVER] Task index=%d\n", task_index);

 	init(HOST, PORT2);

 	switch (task_index){
 		case 0 :
 		task_0();
 		break;
 		case 1 :
 		task_1();
 		break;
 		case 2 :
 		task_2();
 		break;
 		case 3 :
 		task_3();
 		break;
 		case 4 :
 		task_4();
 		break;
 		default:
 		printf("Bad task\n");
 		return 0;
 	}
 	return 0;
 }
