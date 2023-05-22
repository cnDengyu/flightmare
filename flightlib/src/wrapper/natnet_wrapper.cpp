#include "microsim/natnet_wrapper.hpp"
#include <functional>
#include <iostream>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <inttypes.h>
#include <stdio.h>

using namespace std;
namespace py = pybind11;
using namespace py::literals;

namespace microsim {

// static variables

const ConnectionType NatNetManager::_DefaultConnectionType =
  ConnectionType_Multicast;
std::vector<sNatNetDiscoveredServer> NatNetManager::_discoveredServers;
char NatNetManager::_discoveredMulticastGroupAddr[kNatNetIpv4AddrStrLenMax] =
  NATNET_DEFAULT_MULTICAST_ADDRESS;
std::map<int, sRigidBody> NatNetManager::_rigidBodyMap;
sServerDescription NatNetManager::_serverDescription;

// static functions

void NATNET_CALLCONV NatNetManager::DataHandler(sFrameOfMocapData* data,
                                                void* pUserData) {
  NatNetClient* pClient = (NatNetClient*)pUserData;

  // Software latency here is defined as the span of time between:
  //   a) The reception of a complete group of 2D frames from the camera system
  //   (CameraDataReceivedTimestamp)
  // and
  //   b) The time immediately prior to the NatNet frame being transmitted over
  //   the network (TransmitTimestamp)
  //
  // This figure may appear slightly higher than the "software latency" reported
  // in the Motive user interface, because it additionally includes the time
  // spent preparing to stream the data via NatNet.
  const uint64_t softwareLatencyHostTicks =
    data->TransmitTimestamp - data->CameraDataReceivedTimestamp;
  const double softwareLatencyMillisec =
    (softwareLatencyHostTicks * 1000) /
    static_cast<double>(_serverDescription.HighResClockFrequency);

  // Transit latency is defined as the span of time between Motive transmitting
  // the frame of data, and its reception by the client (now). The
  // SecondsSinceHostTimestamp method relies on NatNetClient's internal clock
  // synchronization with the server using Cristian's algorithm.
  const double transitLatencyMillisec =
    pClient->SecondsSinceHostTimestamp(data->TransmitTimestamp) * 1000.0;

  int i = 0;

  /*
  printf("FrameID : %d\n", data->iFrame);
  printf("Timestamp : %3.2lf\n", data->fTimestamp);
  printf("Software latency : %.2lf milliseconds\n", softwareLatencyMillisec);
  */

  // Only recent versions of the Motive software in combination with ethernet
  // camera systems support system latency measurement. If it's unavailable (for
  // example, with USB camera systems, or during playback), this field will be
  // zero.
  const bool bSystemLatencyAvailable = data->CameraMidExposureTimestamp != 0;
  // const bool bSystemLatencyAvailable = true;

  if (bSystemLatencyAvailable) {
    // System latency here is defined as the span of time between:
    //   a) The midpoint of the camera exposure window, and therefore the
    //   average age of the photons (CameraMidExposureTimestamp)
    // and
    //   b) The time immediately prior to the NatNet frame being transmitted
    //   over the network (TransmitTimestamp)
    const uint64_t systemLatencyHostTicks =
      data->TransmitTimestamp - data->CameraMidExposureTimestamp;
    const double systemLatencyMillisec =
      (systemLatencyHostTicks * 1000) /
      static_cast<double>(_serverDescription.HighResClockFrequency);

    // Client latency is defined as the sum of system latency and the transit
    // time taken to relay the data to the NatNet client. This is the
    // all-inclusive measurement (photons to client processing).
    const double clientLatencyMillisec =
      pClient->SecondsSinceHostTimestamp(data->CameraMidExposureTimestamp) *
      1000.0;

    // You could equivalently do the following (not accounting for time elapsed
    // since we calculated transit latency above):
    // const double clientLatencyMillisec = systemLatencyMillisec +
    // transitLatencyMillisec;

    /*
    printf("System latency : %.2lf milliseconds\n", systemLatencyMillisec);
    printf(
      "Total client latency : %.2lf milliseconds (transit time +%.2lf ms)\n",
      clientLatencyMillisec, transitLatencyMillisec);
      */
  } else {
    // printf("Transit latency : %.2lf milliseconds\n", transitLatencyMillisec);
  }

  // FrameOfMocapData params
  bool bIsRecording = ((data->params & 0x01) != 0);
  bool bTrackedModelsChanged = ((data->params & 0x02) != 0);
  if (bIsRecording) printf("RECORDING\n");
  if (bTrackedModelsChanged) printf("Models Changed.\n");


  // timecode - for systems with an eSync and SMPTE timecode generator -
  // decode to values
  int hour, minute, second, frame, subframe;
  NatNet_DecodeTimecode(data->Timecode, data->TimecodeSubframe, &hour, &minute,
                        &second, &frame, &subframe);
  // decode to friendly string
  char szTimecode[128] = "";
  NatNet_TimecodeStringify(data->Timecode, data->TimecodeSubframe, szTimecode,
                           128);
  // printf("Timecode : %s\n", szTimecode);

  // Rigid Bodies
  // printf("Rigid Bodies [Count=%d]\n", data->nRigidBodies);
  for (i = 0; i < data->nRigidBodies; i++) {
    // params
    // 0x01 : bool, rigid body was successfully tracked in this frame
    bool bTrackingValid = data->RigidBodies[i].params & 0x01;

    /*
    printf("Rigid Body [ID=%d  Error=%3.2f  Valid=%d]\n",
           data->RigidBodies[i].ID, data->RigidBodies[i].MeanError,
           bTrackingValid);
    printf("\tx\ty\tz\tqx\tqy\tqz\tqw\n");
    printf("\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n",
           data->RigidBodies[i].x, data->RigidBodies[i].y,
           data->RigidBodies[i].z, data->RigidBodies[i].qx,
           data->RigidBodies[i].qy, data->RigidBodies[i].qz,
           data->RigidBodies[i].qw);
           */
    sRigidBodyData rbd = data->RigidBodies[i];
    // cout << "get rbd\n";
    map<int, sRigidBody>::iterator rbm = _rigidBodyMap.find(rbd.ID);
    // cout << "get rbm\n";
    rbm->second.x = rbd.x;
    rbm->second.y = rbd.y;
    rbm->second.z = rbd.z;
    rbm->second.qw = rbd.qw;
    rbm->second.qx = rbd.qx;
    rbm->second.qy = rbd.qy;
    rbm->second.qz = rbd.qz;
    // cout << rbd.x << ' ' << rbd.y << ' ' << rbd.z << '\n';
  }
}

void NATNET_CALLCONV NatNetManager::MessageHandler(Verbosity msgType,
                                                   const char* msg) {}

void NATNET_CALLCONV NatNetManager::ServerDiscoveredCallback(
  const sNatNetDiscoveredServer* pDiscoveredServer, void* pUserContext) {
  char serverHotkey = '.';
  if (_discoveredServers.size() < 9) {
    serverHotkey = static_cast<char>('1' + _discoveredServers.size());
  }

  printf("[%c] %s %d.%d at %s ", serverHotkey,
         pDiscoveredServer->serverDescription.szHostApp,
         pDiscoveredServer->serverDescription.HostAppVersion[0],
         pDiscoveredServer->serverDescription.HostAppVersion[1],
         pDiscoveredServer->serverAddress);

  if (pDiscoveredServer->serverDescription.bConnectionInfoValid) {
    printf("(%s)\n", pDiscoveredServer->serverDescription.ConnectionMulticast
                       ? "multicast"
                       : "unicast");
  } else {
    printf(
      "(WARNING: Legacy server, could not autodetect settings. Auto-connect "
      "may not work reliably.)\n");
  }

  _discoveredServers.push_back(*pDiscoveredServer);
}

// member functions

NatNetManager::NatNetManager() { createClient("127.0.0.1", "127.0.0.1"); }

NatNetManager::NatNetManager(const std::string& serverAddress,
                             const std::string& localAddress) {
  createClient(serverAddress, localAddress);
}

NatNetManager::~NatNetManager() {}

int NatNetManager::createClient(string serverAddress, string localAddress) {

  // print version info
  unsigned char ver[4];
  NatNet_GetVersion(ver);
  printf("NatNet Sample Client (NatNet ver. %d.%d.%d.%d)\n", ver[0], ver[1],
         ver[2], ver[3]);

  // Install logging callback
  NatNet_SetLogCallback(NatNetManager::MessageHandler);

  _pClient->SetFrameReceivedCallback(
    NatNetManager::DataHandler,
    _pClient.get());  // this function will receive data from the server

  _connectParams.connectionType = _DefaultConnectionType;
  _connectParams.serverCommandPort = 1510;
  _connectParams.serverDataPort = 1511;
  _connectParams.serverAddress = serverAddress.c_str();
  _connectParams.localAddress = localAddress.c_str();
  /*
  _connectParams.BitstreamVersion[0] = 0;
  _connectParams.BitstreamVersion[1] = 0;
  _connectParams.BitstreamVersion[2] = 0;
  _connectParams.BitstreamVersion[3] = 0;
  */
  _connectParams.multicastAddress = "239.255.42.99";

  cout << "NatNet Client Created:\n"
       << "Server Address:" << serverAddress.c_str() << '\n'
       << "Local Address:" << localAddress.c_str() << '\n';

  return 0;
}

int NatNetManager::connectClient() {

NatNetDiscoveryHandle discovery;
  NatNet_CreateAsyncServerDiscovery(&discovery,
                                    NatNetManager::ServerDiscoveredCallback);

  const size_t serverIndex = 0;

  cout << "Searching Server...\n";
  while (_discoveredServers.size() == 0) {
    cout << '.';
    _sleep(500);
  }
  cout << '\n';

  const sNatNetDiscoveredServer& discoveredServer =
    _discoveredServers[serverIndex];

  if (discoveredServer.serverDescription.bConnectionInfoValid) {
    // Build the connection parameters.
#ifdef _WIN32
    _snprintf_s(
#else
    snprintf(
#endif
      _discoveredMulticastGroupAddr, sizeof _discoveredMulticastGroupAddr,
      "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "",
      discoveredServer.serverDescription.ConnectionMulticastAddress[0],
      discoveredServer.serverDescription.ConnectionMulticastAddress[1],
      discoveredServer.serverDescription.ConnectionMulticastAddress[2],
      discoveredServer.serverDescription.ConnectionMulticastAddress[3]);

    _connectParams.connectionType =
      discoveredServer.serverDescription.ConnectionMulticast
        ? ConnectionType_Multicast
        : ConnectionType_Unicast;
    _connectParams.serverCommandPort = discoveredServer.serverCommandPort;
    _connectParams.serverDataPort =
      discoveredServer.serverDescription.ConnectionDataPort;
    _connectParams.serverAddress = discoveredServer.serverAddress;
    _connectParams.localAddress = discoveredServer.localAddress;
    _connectParams.multicastAddress = _discoveredMulticastGroupAddr;
  } else {
    // We're missing some info because it's a legacy server.
    // Guess the defaults and make a best effort attempt to connect.
    _connectParams.connectionType = _DefaultConnectionType;
    _connectParams.serverCommandPort = discoveredServer.serverCommandPort;
    _connectParams.serverDataPort = 0;
    _connectParams.serverAddress = discoveredServer.serverAddress;
    _connectParams.localAddress = discoveredServer.localAddress;
    _connectParams.multicastAddress = NULL;
  }

  NatNet_FreeAsyncServerDiscovery(discovery);


  // Release previous server
  _pClient->Disconnect();
  // Init Client and connect to NatNet server
  int retCode = _pClient->Connect(_connectParams);
  if (retCode != ErrorCode_OK) {
    printf("Unable to connect to server.  Error code: %d. Exiting.\n", retCode);
    cout << "connectionType " << _connectParams.connectionType << '\n'
         << "serverCommandPort " << _connectParams.serverCommandPort << '\n'
         << "serverDataPort " << _connectParams.serverDataPort << '\n';
    return ErrorCode_Internal;
  } else {
    // connection succeeded
    void* pResult;
    int nBytes = 0;
    ErrorCode ret = ErrorCode_OK;

    // print server info
    memset(&_serverDescription, 0, sizeof(_serverDescription));
    ret = _pClient->GetServerDescription(&_serverDescription);
    if (ret != ErrorCode_OK || !_serverDescription.HostPresent) {
      printf("Unable to connect to server. Host not present. Exiting.\n");
      return 1;
    }
    printf("\n[SampleClient] Server application info:\n");
    printf("Application: %s (ver. %d.%d.%d.%d)\n", _serverDescription.szHostApp,
           _serverDescription.HostAppVersion[0],
           _serverDescription.HostAppVersion[1],
           _serverDescription.HostAppVersion[2],
           _serverDescription.HostAppVersion[3]);
    printf("NatNet Version: %d.%d.%d.%d\n", _serverDescription.NatNetVersion[0],
           _serverDescription.NatNetVersion[1],
           _serverDescription.NatNetVersion[2],
           _serverDescription.NatNetVersion[3]);
    printf("Client IP:%s\n", _connectParams.localAddress);
    printf("Server IP:%s\n", _connectParams.serverAddress);
    printf("Server Name:%s\n", _serverDescription.szHostComputerName);

    // get mocap frame rate
    ret = _pClient->SendMessageAndWait("FrameRate", &pResult, &nBytes);
    if (ret == ErrorCode_OK) {
      float fRate = *((float*)pResult);
      printf("Mocap Framerate : %3.2f\n", fRate);
    } else
      printf("Error getting frame rate.\n");

    // get # of analog samples per mocap frame of data
    ret = _pClient->SendMessageAndWait("AnalogSamplesPerMocapFrame", &pResult,
                                       &nBytes);
    if (ret == ErrorCode_OK) {
      _analogSamplesPerMocapFrame = *((int*)pResult);
      printf("Analog Samples Per Mocap Frame : %d\n",
             _analogSamplesPerMocapFrame);
    } else
      printf("Error getting Analog frame rate.\n");
  }

  return ErrorCode_OK;
}

void NatNetManager::resetClient() {
  int iSuccess;

  printf("\n\nre-setting Client\n\n.");

  iSuccess = _pClient->Disconnect();
  if (iSuccess != 0) printf("error un-initting Client\n");

  iSuccess = _pClient->Connect(_connectParams);
  if (iSuccess != 0) printf("error re-initting Client\n");
}

void NatNetManager::closeClient() {
  int iSuccess;
  iSuccess = _pClient->Disconnect();
  if (iSuccess != 0) printf("error un-initting Client\n");
}

int NatNetManager::getDescription() {
  int iResult = 0;
  _rigidBodyDescriptions.clear();
  // Send/receive test request
  void* response;
  int nBytes;
  printf("[SampleClient] Sending Test Request\n");
  iResult = _pClient->SendMessageAndWait("TestRequest", &response, &nBytes);
  if (iResult == ErrorCode_OK) {
    printf("[SampleClient] Received: %s\n", (char*)response);
  }

  // Retrieve Data Descriptions from Motive
  printf("\n\n[SampleClient] Requesting Data Descriptions...\n");
  sDataDescriptions* pDataDefs = NULL;
  iResult = _pClient->GetDataDescriptionList(&pDataDefs);
  if (iResult != ErrorCode_OK || pDataDefs == NULL) {
    printf("[SampleClient] Unable to retrieve Data Descriptions.\n");
  } else {
    printf("[SampleClient] Received %d Data Descriptions:\n",
           pDataDefs->nDataDescriptions);
    for (int i = 0; i < pDataDefs->nDataDescriptions; i++) {
      printf("Data Description # %d (type=%d)\n", i,
             pDataDefs->arrDataDescriptions[i].type);
      if (pDataDefs->arrDataDescriptions[i].type == Descriptor_MarkerSet) {
        // MarkerSet
        sMarkerSetDescription* pMS =
          pDataDefs->arrDataDescriptions[i].Data.MarkerSetDescription;
        printf("MarkerSet Name : %s\n", pMS->szName);
        for (int i = 0; i < pMS->nMarkers; i++)
          printf("%s\n", pMS->szMarkerNames[i]);

      } else if (pDataDefs->arrDataDescriptions[i].type ==
                 Descriptor_RigidBody) {
        // RigidBody
        sRigidBodyDescription* pRB =
          pDataDefs->arrDataDescriptions[i].Data.RigidBodyDescription;
        _rigidBodyDescriptions.push_back(*pRB);
        printf("RigidBody Name : %s\n", pRB->szName);
        printf("RigidBody ID : %d\n", pRB->ID);
        printf("RigidBody Parent ID : %d\n", pRB->parentID);
        printf("Parent Offset : %3.2f,%3.2f,%3.2f\n", pRB->offsetx,
               pRB->offsety, pRB->offsetz);

        if (pRB->MarkerPositions != NULL && pRB->MarkerRequiredLabels != NULL) {
          for (int markerIdx = 0; markerIdx < pRB->nMarkers; ++markerIdx) {
            const MarkerData& markerPosition = pRB->MarkerPositions[markerIdx];
            const int markerRequiredLabel =
              pRB->MarkerRequiredLabels[markerIdx];

            printf("\tMarker #%d:\n", markerIdx);
            printf("\t\tPosition: %.2f, %.2f, %.2f\n", markerPosition[0],
                   markerPosition[1], markerPosition[2]);

            if (markerRequiredLabel != 0) {
              printf("\t\tRequired active label: %d\n", markerRequiredLabel);
            }
          }
        }
      } else if (pDataDefs->arrDataDescriptions[i].type ==
                 Descriptor_Skeleton) {
        // Skeleton
        sSkeletonDescription* pSK =
          pDataDefs->arrDataDescriptions[i].Data.SkeletonDescription;
        printf("Skeleton Name : %s\n", pSK->szName);
        printf("Skeleton ID : %d\n", pSK->skeletonID);
        printf("RigidBody (Bone) Count : %d\n", pSK->nRigidBodies);
        for (int j = 0; j < pSK->nRigidBodies; j++) {
          sRigidBodyDescription* pRB = &pSK->RigidBodies[j];
          printf("  RigidBody Name : %s\n", pRB->szName);
          printf("  RigidBody ID : %d\n", pRB->ID);
          printf("  RigidBody Parent ID : %d\n", pRB->parentID);
          printf("  Parent Offset : %3.2f,%3.2f,%3.2f\n", pRB->offsetx,
                 pRB->offsety, pRB->offsetz);
        }
      } else if (pDataDefs->arrDataDescriptions[i].type ==
                 Descriptor_ForcePlate) {
        // Force Plate
        sForcePlateDescription* pFP =
          pDataDefs->arrDataDescriptions[i].Data.ForcePlateDescription;
        printf("Force Plate ID : %d\n", pFP->ID);
        printf("Force Plate Serial : %s\n", pFP->strSerialNo);
        printf("Force Plate Width : %3.2f\n", pFP->fWidth);
        printf("Force Plate Length : %3.2f\n", pFP->fLength);
        printf("Force Plate Electrical Center Offset (%3.3f, %3.3f, %3.3f)\n",
               pFP->fOriginX, pFP->fOriginY, pFP->fOriginZ);
        for (int iCorner = 0; iCorner < 4; iCorner++)
          printf("Force Plate Corner %d : (%3.4f, %3.4f, %3.4f)\n", iCorner,
                 pFP->fCorners[iCorner][0], pFP->fCorners[iCorner][1],
                 pFP->fCorners[iCorner][2]);
        printf("Force Plate Type : %d\n", pFP->iPlateType);
        printf("Force Plate Data Type : %d\n", pFP->iChannelDataType);
        printf("Force Plate Channel Count : %d\n", pFP->nChannels);
        for (int iChannel = 0; iChannel < pFP->nChannels; iChannel++)
          printf("\tChannel %d : %s\n", iChannel,
                 pFP->szChannelNames[iChannel]);
      } else if (pDataDefs->arrDataDescriptions[i].type == Descriptor_Device) {
        // Peripheral Device
        sDeviceDescription* pDevice =
          pDataDefs->arrDataDescriptions[i].Data.DeviceDescription;
        printf("Device Name : %s\n", pDevice->strName);
        printf("Device Serial : %s\n", pDevice->strSerialNo);
        printf("Device ID : %d\n", pDevice->ID);
        printf("Device Channel Count : %d\n", pDevice->nChannels);
        for (int iChannel = 0; iChannel < pDevice->nChannels; iChannel++)
          printf("\tChannel %d : %s\n", iChannel,
                 pDevice->szChannelNames[iChannel]);
      } else if (pDataDefs->arrDataDescriptions[i].type == Descriptor_Camera) {
        // Camera
        sCameraDescription* pCamera =
          pDataDefs->arrDataDescriptions[i].Data.CameraDescription;
        printf("Camera Name : %s\n", pCamera->strName);
        printf("Camera Position (%3.2f, %3.2f, %3.2f)\n", pCamera->x,
               pCamera->y, pCamera->z);
        printf("Camera Orientation (%3.2f, %3.2f, %3.2f, %3.2f)\n", pCamera->qx,
               pCamera->qy, pCamera->qz, pCamera->qw);
      } else {
        printf("Unknown data type.\n");
        // Unknown
      }
    }

    // Construst the rigid body map for position query
    constructRigidBodyMap();
  }
  return iResult;
}

void NatNetManager::constructRigidBodyMap() { 
  for (auto rb : _rigidBodyDescriptions) {
    sRigidBody b;
    b.name = string(rb.szName);
    _rigidBodyMap.insert(pair<int, sRigidBody>(rb.ID, b));
  }
}

py::list NatNetManager::getRigidBodyList() { 
  py::list result;
  for (auto rb : _rigidBodyMap) {
    py::list position = py::list();
    position.append(rb.second.x);
    position.append(rb.second.y);
    position.append(rb.second.z);
    py::list quaternion = py::list();
    quaternion.append(rb.second.qw);
    quaternion.append(rb.second.qx);
    quaternion.append(rb.second.qy);
    quaternion.append(rb.second.qz);
    py::dict rbd = py::dict(
        "id"_a = rb.first ,
        "name"_a = rb.second.name, 
        "position"_a = position, 
        "quaternion"_a = quaternion);
    result.append(rbd);
  }
  return result; 
}

}  // namespace microsim
