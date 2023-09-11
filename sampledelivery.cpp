/*
Copyright 2020 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "common.h"
#include "sampledelivery.h"
#include <thread>

static int delivery_id = 0;

int FileSampleDelivery::DeliverSample(Sample *sample)
{
  return sample->Save(filename.c_str());
}

SHMSampleDelivery::SHMSampleDelivery(char *name, size_t size)
{
  shmobj.Open(name, size);
  shm = shmobj.GetData();
}

SHMSampleDelivery::~SHMSampleDelivery()
{
  shmobj.Close();
}

int SHMSampleDelivery::DeliverSample(Sample *sample)
{
  uint32_t *size_ptr = (uint32_t *)shm;
  unsigned char *data_ptr = shm + 4;
  *size_ptr = (uint32_t)sample->size;
  memcpy(data_ptr, sample->bytes, sample->size);
  return 1;
}

NetworkSampleDelivery::NetworkSampleDelivery(char *address, int port, int init_time)
{
  this->address = address;
  this->port = port;
  this->first_time = true;
  this->init_time = init_time;
}
int NetworkSampleDelivery::DeliverSample(Sample *sample)
{
  // std::thread t(NetworkSampleDelivery::RepeatSendTcp, sample);
  if (!this->first_time)
  {
    this->t.join();
  }
  this->t = std::thread(&NetworkSampleDelivery::RepeatSendTcp, this, sample, false);
  // this->t.detach();
  return 1;
}

void clearRecv(SOCKET delivery_socket)
{
  int bytes_received = 0;
  char buf[1024];
  do
  {
    bytes_received = recv(delivery_socket, buf, 1024, 0);

  } while (bytes_received > 0);
}
void NetworkSampleDelivery::RepeatSendTcp(Sample *sample, bool retry)
{
  static struct sockaddr_in si_other;
  static int slen = sizeof(si_other);
  static WSADATA wsa;
  SOCKET delivery_socket;

  // printf("new send tcp thread %d\n", delivery_id++);
  // Sleep(2000);
  if (this->first_time)
  {
    this->first_time = false;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      FATAL("WSAStartup failed. Error Code : %d", WSAGetLastError());

    // setup address structure
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(this->port);
    si_other.sin_addr.S_un.S_addr = inet_addr((char *)this->address);
  }

  /* In case of TCP we need to open a socket each time we want to establish
   * connection. In theory we can keep connections always open but it might
   * cause our target behave differently (probably there are a bunch of
   * applications where we should apply such scheme to trigger interesting
   * behavior).
   */
  if ((delivery_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == SOCKET_ERROR)
  {
    FATAL("socket() failed with error code : %d", WSAGetLastError());
  }

  // Connect to server.
  int count = 0;
  int c = 0;
  int connect_status;

  do
  {
    connect_status = connect(delivery_socket, (SOCKADDR *)&si_other, slen);
    if (connect_status == SOCKET_ERROR)
    {
      // target is not ready yet, let it initialize
      Sleep(this->init_time);
    }
    else
    {
      // printf("connected!\n");
      break;
    }
    if (count++ > 1)
      FATAL("too many retried: connect() failed with error code : %d\n", c);
  } while (true);
  // printf("connected!\n");
  if (connect_status == SOCKET_ERROR)
  {
    // if (!retry)
    // {
    //   printf("retry connect!\n");
    //   // shutdown the connection since no more data will be sent
    //   // if (shutdown(delivery_socket, 0x2 /*SD_SEND*/) == SOCKET_ERROR)
    //   //   FATAL("retry shutdown failed with error: %d\n", WSAGetLastError());
    //   // // close the socket to avoid consuming much resources
    //   // if (closesocket(delivery_socket) == SOCKET_ERROR)
    //   //   FATAL("retry closesocket failed with error: %d\n", WSAGetLastError());
    //   // printf("close socket success\n");
    //   this->RepeatSendTcp(sample, true);
    //   return;
    // }
    // else
    // FATAL("after retry connect() failed with error code : %d", WSAGetLastError());
    FATAL("connect() failed with error code : %d\n", c);
  }
  // Send our buffer

  if (send(delivery_socket, sample->bytes, (int)sample->size, 0) == SOCKET_ERROR)
    // if (!retry)
    // {
    //   printf("retry!\n");
    //   // shutdown the connection since no more data will be sent
    //   if (shutdown(delivery_socket, 0x2 /*SD_SEND*/) == SOCKET_ERROR)
    //     FATAL("retry shutdown failed with error: %d\n", WSAGetLastError());
    //   // close the socket to avoid consuming much resources
    //   if (closesocket(delivery_socket) == SOCKET_ERROR)
    //     FATAL("retry closesocket failed with error: %d\n", WSAGetLastError());
    //   // printf("close socket success\n");
    //   this->RepeatSendTcp(sample, true);
    //   return;
    // }
    // else
    FATAL("send() failed with error code : %d", WSAGetLastError());
  NetworkSampleDelivery::clearRecv(delivery_socket);

  // shutdown the connection since no more data will be sent
  if (shutdown(delivery_socket, 0x2 /*SD_SEND*/) == SOCKET_ERROR)
    FATAL("shutdown failed with error: %d\n", WSAGetLastError());
  // close the socket to avoid consuming much resources
  if (closesocket(delivery_socket) == SOCKET_ERROR)
    FATAL("closesocket failed with error: %d\n", WSAGetLastError());
  // printf("close socket success %d\n", delivery_id);
}
NetworkSampleDelivery::~NetworkSampleDelivery()
{
}
