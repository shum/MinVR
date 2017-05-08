#ifndef VRNETSERVER_H
#define VRNETSERVER_H

#include "VRNetInterface.h"

#ifndef WIN32
  #include <netinet/tcp.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <sys/wait.h>
  #include <signal.h>
#endif

#include <math/VRMath.h>
#include <config/VRDataIndex.h>

namespace MinVR {

class VRNetServer : public VRNetInterface {
 public:

  VRNetServer(const std::string &listenPort, int numExpectedClients);
  ~VRNetServer();

  VRDataQueue::serialData
    syncEventDataAcrossAllNodes(VRDataQueue::serialData eventData);
  VRDataQueue::serialData syncEventDataAcrossAllNodes();

  void syncSwapBuffersAcrossAllNodes();
  // NOTE below method for testing
  void syncSwapBuffersAcrossAllNodes_test();


  int receiveall(unsigned char *buf, int len) {
    return VRNetInterface::receiveall(_clientSocketFDs[0], buf, len);
  }

  void waitForAndReceiveSwapBuffersRequestAcrossAllNodes();

 private:

  std::vector<SOCKET> _clientSocketFDs;

};

}

#endif
