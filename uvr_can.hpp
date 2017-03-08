// License: Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported (CC BY-NC-ND 3.0)

#ifndef UVR_CAN_H
#define UVR_CAN_H

#include <iostream>
#include <fstream>
#include <string>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
//#include <boost/chrono.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

extern  "C" { 
#include <linux/can.h>
#include <linux/can/raw.h>

#include <string.h>
#include <stdio.h> 

#include <canopen/canopen.h>
#include <canopen/canopen-com.h>

#include <time.h>
#include <stdlib.h>
#include <inttypes.h>
}


#define array_size 64
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define DEBUG 0
#define RETRY 3
#define AKTIVIERUNG 0x00
#define DEAKTIVIERUNG 0x01
#define UVR1611 0x80
#define WAITONERROR 50 // mikrosekunden


extern boost::mutex mtx_;

extern int holeZeit(int, uint16_t);
extern int leseAusgaenge(int, uint16_t);
extern int leseWerte(int, uint16_t);
extern int leseMeldung(int, uint16_t);
void pdoHelper(int, int, int, uint8_t);
int isNMTnodeguarding(canopen_frame_t *frame);
int verbindungsAufbau(int, int, int);
int verbindungsAbbau(int, int, int);
int isUVR(int, int, int);
void selektiereUVRs(int, int, int []);
void canHeartBeatFunc(int, int);
int leseWerteUVRThreadFunc(int, int, int []);

int leseBezeichnungEingaenge(int sock, uint16_t node);
int leseBezeichnungAusgaenge(int sock, uint16_t node);

int mycanopen_sdo_upload_exp(int sock, uint8_t node, uint16_t index, uint8_t subindex, uint32_t *data);

// in klasse abgebildet
void getSocket(std::string, int *);

class Uvr_Can {
  private:
    int sock_;
    int is_initialized_;
    int self_node_id_;
    boost::mutex can_if_mtx_;

    int verbindungsAufbau_c(int);
    int verbindungsAbbau_c(int);

  public:
    Uvr_Can(): sock_(-1), is_initialized_(0), self_node_id_(44) {};
    int getSocket_c(std::string);
    int is_initialized();
    int heartBeatFunc();
    int heartBeatThreadFunc(int);
    int scanBus(int[]);
    int isUVR_c(int);
    int leseWerte_c(int);
    int selektiereUVRs_c(int[]);
    int leseBeschreibung_c(int);
    int printKnoten(int);
};

#endif
