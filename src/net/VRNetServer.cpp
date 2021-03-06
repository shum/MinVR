#include <net/VRNetServer.h>

using namespace std;

namespace MinVR {

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 100	 // how many pending connections queue will hold

#ifndef WIN32
void sigchld_handler(int s) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}
#endif

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



VRNetServer::VRNetServer(const std::string &listenPort, int numExpectedClients)
{

#ifdef WIN32  // Winsock implementation

  WSADATA wsaData;

  // listen on sock_fd, new connection on new_fd
  SOCKET sockfd = INVALID_SOCKET;
  SOCKET new_fd = INVALID_SOCKET;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  const char yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  rv = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (rv != 0) {
    cerr << "WSAStartup failed with error: " << rv << endl;
    exit(1);
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, listenPort.c_str(), &hints, &servinfo)) != 0) {
    cerr << "getaddrinfo() failed with error: " << rv << endl;
    WSACleanup();
    exit(1);
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
      cerr << "socket() failed with error: " << WSAGetLastError() << endl;
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERROR) {
      cerr << "setsockopt() failed with error: " << WSAGetLastError() << endl;
      closesocket(sockfd);
      WSACleanup();
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) {
      closesocket(sockfd);
      sockfd = INVALID_SOCKET;
      cerr << "bind() failed with error: " << WSAGetLastError() << endl;
      continue;
    }

    break;
  }

  if (p == NULL) {
    cerr << "server: failed to bind" << endl;
    //return 2;
    exit(2);
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (listen(sockfd, BACKLOG) == SOCKET_ERROR) {
    cerr << "listen failed with errror: " << WSAGetLastError() << endl;
    closesocket(sockfd);
    WSACleanup();
    exit(1);
  }

  // Should we do the "reap all dead processes" as in the linux implementation below?

  printf("server: waiting for connections...\n");

  int numConnected = 0;
  while (numConnected < numExpectedClients) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == INVALID_SOCKET) {
      cerr << "server: got invalid socket while accepting connection" << endl;
      continue;
    }

    // Disable Nagle's algorithm on the client's socket
    char value = 1;
    setsockopt(new_fd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

    numConnected++;
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection %d from %s\n", numConnected, s);

    _clientSocketFDs.push_back(new_fd);
  }


#else  // BSD sockets implementation

  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, listenPort.c_str(), &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    //return 1;
    exit(1);
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    //return 2;
    exit(2);
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  int numConnected = 0;
  while (numConnected < numExpectedClients) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    // Disable Nagle's algorithm on the client's socket
    char value = 1;
    setsockopt(new_fd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

    numConnected++;
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection %d from %s\n", numConnected, s);

    _clientSocketFDs.push_back(new_fd);
  }

#endif
}

VRNetServer::~VRNetServer()
{
  for (std::vector<SOCKET>::iterator i=_clientSocketFDs.begin(); i < _clientSocketFDs.end(); i++) {
	#ifdef WIN32
      closesocket(*i);
    #else
      close(*i);
    #endif
  }

#ifdef WIN32
  WSACleanup();
#endif
}


// Wait for and receive an eventData message from every client, add
// them together and send them out again.
VRDataQueue::serialData
VRNetServer::syncEventDataAcrossAllNodes(VRDataQueue::serialData eventData) {

	// std::cout << eventData << std::endl;
  VRDataQueue dataQueue = VRDataQueue(eventData);
	// std::cout << dataQueue.serialize() << std::endl;

  // TODO: rather than a for loop, could use a select() system call
  // here (I think) to figure out which socket is ready for a read in
  // the situation where one client is ready but other(s) are not
  for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
       itr < _clientSocketFDs.end(); itr++) {
    VRDataQueue::serialData ed = waitForAndReceiveEventData(*itr);
		// std::cout << ed << std::endl;
    dataQueue.addSerializedQueue(ed);
		// std::cout << dataQueue.serialize() << std::endl;
  }

  VRDataQueue::serialData dq = dataQueue.serialize();
	// std::cout << dq << std::endl;
  // 2. send new combined inputEvents array out to all clients
  for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
       itr < _clientSocketFDs.end(); itr++) {
    sendEventData(*itr, dq);
  }

  return dq;
}

// This variant is not used by the server, but is part of the net
// interface definition for convenience.  Empty definition here to
// satisfy the compiler.
VRDataQueue::serialData
VRNetServer::syncEventDataAcrossAllNodes() {

  VRDataQueue::serialData out = "";
  return out;

}

void VRNetServer::syncSwapBuffersAcrossAllNodes() {
  // 1. wait for, receive, and parse a swap_buffers_request message
  // from every client

  // TODO: rather than a for loop could use a select() system call here (I think) to figure out which socket is ready for a read in the situation where 1 is ready but other(s) are not
  for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
       itr < _clientSocketFDs.end(); itr++) {
    waitForAndReceiveSwapBuffersRequest(*itr);
  }

  // 2. send a swap_buffers_now message to every client
  for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
       itr < _clientSocketFDs.end(); itr++) {
    sendSwapBuffersNow(*itr);
  }
}


void VRNetServer::syncSwapBuffersAcrossAllNodes_test() {
/*
	NOTE deprecated select() in favor of poll() for reusability of struct
	const int TOTALSOCKETS = _clientSocketFDs.size();
	int maxfd, readcounter;
	fd_set readfds;
	struct timeval tv;

	// clears the set
	FD_ZERO(&readfds);
	maxfd = 0;

	// initialize the read set
  for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
       itr < _clientSocketFDs.end(); itr++) {
		FD_SET(*itr, &readfds);
		maxfd = (maxfd > *itr) ? maxfd : *itr;
	}

	// timeout 10.5 seconds
	tv.tv_sec = 10;
	tv.tv_usec = 500000;

	readcounter = 0;

	while (readcounter < TOTALSOCKETS) {
		// check for which sockets can be read
		int result = select(maxfd+1, &readfds, NULL, NULL, &tv);
		if (result == -1) {
		   std::cerr << "Error select()" << std::endl;
		} else if (result == 0) {
			std::cout << "Timeout occurred! No data after 10.5 seconds." << std::endl;
		} else {
			for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
		       itr < _clientSocketFDs.end(); itr++) {
					std::cout << *itr << '\n';
		      if (FD_ISSET(*itr, &readfds)) {
						// while (receivedID != VRNetInterface::SWAP_BUFFERS_REQUEST_MSG) {
							unsigned char receivedID = 0x0;
					    int status = VRNetInterface::receiveall(*itr, &receivedID, 1);

							std::cout << *itr << status << readcounter << '\n';

					    if (status == -1) {
					      std::cerr << "NetInterface error: receiveall failed." << std::endl;
					      exit(1);
					    }
					    else if ((status == 1) && (receivedID != VRNetInterface::SWAP_BUFFERS_REQUEST_MSG)) {
					      std::cerr << "NetInterface error, unexpected message. Received: " << (int)receivedID << std::endl;
					    } else {
								readcounter++;
							}
					  // }
		      } else {
						std::cout << "select not ready" << readcounter << '\n';
					}
		   }
		}
	}
*/
	const int TOTALSOCKETS = _clientSocketFDs.size();
	struct pollfd readfds[TOTALSOCKETS];

	// initialize number of file descriptors and poll timeout
	int nfds = 0, timeout = 10000;
	// initialize the read set
	for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
			 itr < _clientSocketFDs.end(); itr++) {
		readfds[nfds].fd = *itr;
		readfds[nfds].events = POLLIN;
		nfds++;
	}

	// Loop waiting for incoming data on any of the connected sockets
	int readcounter = 0;
	do {
		printf("Waiting on poll() ... \n");
		// Call poll() and wait 10 seconds for it to complete
		int ret = poll(readfds, nfds, timeout);

		// check to see if the poll call failed
		if (ret < 0) {
			std::cerr << "poll() failed" << std::endl;
			break;
		}
		// check to see if the time out expired
		if (ret == 0) {
			std::cout << "poll() timed out" << std::endl;
			break;
		}

		// one or more fds are readable, need to determine which ones
		for (int i = 0; i < nfds; i++) {
			// loop to find the descriptors that returned POLLIN
			if (readfds[i].revents == 0) {
				continue;
			}

			// if revents is not POLLIN, it's an unexpected result
			if (readfds[i].revents != POLLIN) {
				std::cout << "poll error, unexpected revents. Received: " << (readfds[i].revents & POLLERR) << (readfds[i].revents & POLLHUP) << std::endl;
				exit(0);
				break;
			}

			// Receive all incoming data on this socket before we loop back and call poll again
			printf("FD %d is readable\n", readfds[i].fd);
			unsigned char receivedID = 0x0;
			int status = VRNetInterface::receiveall(readfds[i].fd, &receivedID, 1);
			std::cout << readfds[i].fd << status << readcounter << '\n';

			if (status == -1) {
				std::cerr << "NetInterface error: receiveall failed." << std::endl;
				exit(1);
			}
			else if ((status == 1) && (receivedID != VRNetInterface::SWAP_BUFFERS_REQUEST_MSG)) {
				std::cerr << "NetInterface error, unexpected message. Received: " << (int)receivedID << std::endl;
			}
			// data was received and byte was a swap buffer request
			else {
				//after reading from fd, remove it from the readfds set
				readfds[i].fd = -1;
				readcounter++;
			}
		}
	} while (readcounter < TOTALSOCKETS);

	// 2. send a swap_buffers_now message to every client
  for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
       itr < _clientSocketFDs.end(); itr++) {
    sendSwapBuffersNow(*itr);
	}

	std::cout << "SUCCESS" << '\n';
}

void VRNetServer::waitForAndReceiveSwapBuffersRequestAcrossAllNodes() {
	// NOTE Testing method to check if client sendSwapBuffersRequest and server waitForAndReceiveSwapBuffersRequest work
  for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
       itr < _clientSocketFDs.end(); itr++) {
    waitForAndReceiveSwapBuffersRequest(*itr);
  }
}

} // end namespace MinVR
