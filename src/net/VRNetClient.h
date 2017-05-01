#ifndef VRNETCLIENT_H
#define VRNETCLIENT_H

#include "VRNetInterface.h"

#ifndef WIN32
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
#endif

namespace MinVR {

class VRNetClient : public VRNetInterface {
 public:

  VRNetClient(const std::string &serverIP, const std::string &serverPort);
  ~VRNetClient();

  VRDataQueue::serialData
    syncEventDataAcrossAllNodes(VRDataQueue::serialData eventData);

  void syncSwapBuffersAcrossAllNodes();

  // public sendall method, copy from VRNetInterface.cpp
  int sendall(const unsigned char *buf, int len) {
    return VRNetInterface::sendall(_socketFD, buf, len);
  }

  void sendSwapBuffersRequest() {
    return VRNetInterface::sendSwapBuffersRequest(_socketFD);
  }

 private:

  SOCKET _socketFD;

};


} // end namespace MinVR


#endif
