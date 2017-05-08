#include "net/VRNetClient.h"
#include "config/VRDataIndex.h"
#include "config/VRDataQueue.h"
#include <vector>

// NOTE program to test VRNetClient receiveall
// Intended to be used in tandem with testserver receiveall program
/*
int main() {
	MinVR::VRNetClient client = MinVR::VRNetClient("localhost", "3490");
	unsigned char *buf = new unsigned char[1];
	buf[0] = 1; //NOTE 1 shows up as blank as string
	std::cout << buf << std::endl;
	client.sendall(buf, 1);
	std::cout << buf << std::endl;
}
*/

// NOTE program to demonstrate odd char[] behavior
/*
int main() {
	MinVR::VRNetClient client = MinVR::VRNetClient("localhost", "3490");
	unsigned char veryrandom[1];
	veryrandom[0] = 1; //NOTE 1 shows up as blank as string
	std::cout << veryrandom << std::endl; //always outputs localhost
	client.sendall(veryrandom, 1);
	std::cout << veryrandom << std::endl; //always outputs localhost
}
*/

// NOTE program to test send swap buffers request
// Intended to be used in tandem with testserver waitForAndReceiveSwapBuffersRequestAcrossAllNodes program
/*
int main() {
	MinVR::VRNetClient client = MinVR::VRNetClient("localhost", "3490");
	std::cout << "SEND SWAP BUFFERS REQUEST" << std::endl;
	client.sendSwapBuffersRequest();
}
*/

// NOTE program to test multiple send swap buffers request
// Intended to be used in tandem with testserver waitForAndReceiveSwapBuffersRequestAcrossAllNodes program
/*
int main(int argc, char* argv[]) {
  int defaultchoice = 1;
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

	for (int i=0; i<choice; i++) {
		MinVR::VRNetClient client = MinVR::VRNetClient("localhost", "3490");
		std::cout << "SEND SWAP BUFFERS REQUEST" << std::endl;
		client.sendSwapBuffersRequest();
	}
}
*/

// NOTE program to test launching multiple clients and syncEventDataAcrossAllNodes
/*
int main(int argc, char* argv[]) {
  int defaultchoice = 2; // program by default instantiates 2 clients
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

  std::vector<pid_t> pids(choice);

  // forks n processes; each process connects to server and sends a syncEventDataAcrossAllNodes
  for (int i = 0; i < choice; i++) {
    pids[i] = fork();
    if (pids[i] < 0) {
      printf("fork() faled\n");
    } else if (pids[i] == 0) {
      printf("Child process %d (parent %d) syncEventDataAcrossAllNodes\n", getpid(), getppid());
    	MinVR::VRNetClient client = MinVR::VRNetClient("localhost", "3490");
    	MinVR::VRDataQueue::serialData eventData = client.syncEventDataAcrossAllNodes(std::to_string(i));
      std::cout << "client received: " << eventData << std::endl;
      exit(0);
    }
  }

  // waits for n child processes to finish running
  for (int i = 0; i < choice; ++i) {
    int status;
    while (-1 == waitpid(pids[i], &status, 0));
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        cerr << "Process " << i << " (pid " << pids[i] << ") failed" << endl;
        exit(1);
    }
  }
  std::cout << "SUCCESS" << std::endl;
}
*/

int main(int argc, char* argv[]) {
  int defaultchoice = 2; // program by default instantiates 2 clients
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

  std::vector<pid_t> pids(choice);

  // forks n processes; each process connects to server and sends a syncEventDataAcrossAllNodes
  for (int i = 0; i < choice; i++) {
    pids[i] = fork();
    if (pids[i] < 0) {
      printf("fork() faled\n");
    } else if (pids[i] == 0) {
      printf("Child process %d (parent %d) syncEventDataAcrossAllNodes\n", getpid(), getppid());
    	MinVR::VRNetClient client = MinVR::VRNetClient("localhost", "3490");
      std::cout << "SEND SWAP BUFFERS REQUEST" << std::endl;
    	client.sendSwapBuffersRequest();
      exit(0);
    }
  }

  // waits for n child processes to finish running
  for (int i = 0; i < choice; ++i) {
    int status;
    while (-1 == waitpid(pids[i], &status, 0));
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        cerr << "Process " << i << " (pid " << pids[i] << ") failed" << endl;
        exit(1);
    }
  }
  std::cout << "CLIENT SUCCESS" << std::endl;
}
