#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <liburing.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT_NUMBER 8889
#define MAX_CONNECTIONS 1000
#define QUEUE_DEPTH 256

struct io_uring ring;

enum request_types {
	EVENT_TYPE_ACCEPT
};

struct request {
	int event_type;
};

void fatal_error(char * error_message) {
	perror(error_message);
	exit(1);
}

int create_socket() {
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_fd == -1) {
		fatal_error("Creation of Socket failed");
	}

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT_NUMBER);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(
    		socket_fd, 
    		(struct sockaddr *)(&server_address),
    		sizeof(server_address)
    		)
    		== -1) {
    	fatal_error("bind()");
    }

    if (listen(socket_fd, MAX_CONNECTIONS) == -1) {
    	fatal_error("listen()");
    }



}

void sigint_handler(int signo) {
    printf("^C pressed. Shutting down.\n");
    io_uring_queue_exit(&ring);
    exit(0);
}


void add_accept_request(int server_socket_fd,  
						struct sockaddr_in *client_addr, 
						socklen_t *client_addr_len) {

	struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);

	io_uring_prep_accept(sqe, server_socket_fd,  (struct sockaddr *) client_addr,
						 client_addr_len, 0);
	
	struct request *req = malloc(sizeof(*req));
	req -> event_type = EVENT_TYPE_ACCEPT;

	io_uring_sqe_set_data(sqe, req);
	io_uring_submit(&ring);
}

void main_server_loop(int server_socket_fd) {
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	add_accept_request(server_socket_fd, &client_addr, &client_addr_len);

	while (1) {
			struct io_uring_cqe *cqe;
			int ret_val = io_uring_wait_cqe(&ring, &cqe);
			// waits here till the first connection is sent to server

			struct request *req = (struct request *) (cqe -> user_data);

			if (ret_val < 0) fatal_error("io_uring_wait_cqe");

			if (cqe -> res < 0) {
				fprintf(stderr, "Async request failed: %s for event: %d\n",
							strerror(-cqe -> res), cqe -> event_type);
				exit(1); // necessary to exit?
			}

			switch (req -> event_type) {
				case EVENT_TYPE_ACCEPT:
					


			}






	}
}



int main() {
   	int server_socket_fd = create_socket();

   	signal(SIGINT, sigint_handler);

   	io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

   	main_server_loop(server_socket_fd);

   	return 0; 
}


