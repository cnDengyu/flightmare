#pragma once
#include <NatNetCAPI.h>

#include <memory>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "NatNetClient.h"
#include "NatNetTypes.h"

namespace microsim {

struct sRigidBody {
  std::string name;
  float x, y, z;
  float qx, qy, qz, qw;
};

class NatNetManager {
 public:
  NatNetManager();
  NatNetManager(const std::string& serverAddress, const std::string& localAddress);
  ~NatNetManager();
  int connectClient();
  void resetClient();
  void closeClient();
  int getDescription();
  pybind11::list getRigidBodyList();

 private:
  int createClient(std::string serverAddress, std::string localAddress);
  void constructRigidBodyMap();
  // Establish a NatNet Client connection

  // static callback functions.
  static void NATNET_CALLCONV ServerDiscoveredCallback(
    const sNatNetDiscoveredServer* pDiscoveredServer, void* pUserContext);
  static void NATNET_CALLCONV
  DataHandler(sFrameOfMocapData* data,
              void* pUserData);  // receives data from the server
  static void NATNET_CALLCONV MessageHandler(
    Verbosity msgType, const char* msg);  // receives NatNet error messages

  std::unique_ptr<NatNetClient> _pClient = std::make_unique<NatNetClient>();
  sNatNetClientConnectParams _connectParams;
  int _analogSamplesPerMocapFrame = 0;

  std::vector<sRigidBodyDescription> _rigidBodyDescriptions;

  // static variables
  static const ConnectionType _DefaultConnectionType;
  static std::vector<sNatNetDiscoveredServer> _discoveredServers;
  static char _discoveredMulticastGroupAddr[kNatNetIpv4AddrStrLenMax];
  static std::map<int, sRigidBody> _rigidBodyMap;
  static sServerDescription _serverDescription;
};

}  // namespace microsim
