// License: Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported (CC BY-NC-ND 3.0)


#include "uvr_can.hpp"

int Uvr_Can::selektiereUVRs_c(int akt_knoten[])
{   
    //std::cout << "Selektiere UVRs" << std::endl;

    for(int i = 0; i < array_size; i++) {
       if(akt_knoten[i] == 1) {   
          akt_knoten[i] = this->isUVR_c(i);
       }
    }

    for(int i = 0; i < array_size; i++) {
        if(akt_knoten[i] == 1611) {
                //std::cout << "---UVR Knoten: " << i << std::endl;
                ;
        }
    }
    return 0;
}

int Uvr_Can::printKnoten(int node) {
    std::cout << "\"knoten\": " << node << ", " << std::endl;
    return 0;
}

//public
int Uvr_Can::leseWerte_c(int to_node_id){
    int cob_id = 0;

    //scoped_lock
    boost::mutex::scoped_lock lock(can_if_mtx_);

    //cob_id = verbindungsAufbau(sock_, self_node_id_, to_node_id);
    cob_id = this->verbindungsAufbau_c(to_node_id);

    // hier erfolgt die Arbeit
    std::cout << "{" << std::endl;
    printKnoten(to_node_id);
    holeZeit(sock_, cob_id);
    std::cout << "," << std::endl << "\"ausgaenge\": [" << std::endl;
    leseAusgaenge(sock_, cob_id);

    this->verbindungsAbbau_c(to_node_id);
    
    cob_id = this->verbindungsAufbau_c(to_node_id);

    std::cout << "], " << std::endl << "\"eingaenge\": [" << std::endl;
    leseWerte(sock_, cob_id);
    std::cout << "]} " << std::endl; // << "\"meldung\": \"" << std::endl;
    //leseMeldung(sock_, cob_id);
    //std::cout << "\"" << std::endl << "}";

    //verbindungsAbbau(sock_, self_node_id_, to_node_id);
    this->verbindungsAbbau_c(to_node_id);

    return 0;
}

//public
int Uvr_Can::leseBeschreibung_c(int to_node_id) {
    int cob_id = 0;

    //scoped_lock
    boost::mutex::scoped_lock lock(can_if_mtx_);

    //cob_id = verbindungsAufbau(sock_, self_node_id_, to_node_id);
    cob_id = this->verbindungsAufbau_c(to_node_id);

    std::cout << "{" << std::endl;
    printKnoten(to_node_id);
    std::cout << "\"bezeichner\": [";
    // hier erfolgt die Arbeit
    leseBezeichnungEingaenge(sock_, cob_id);
    std::cout << ",";
    this->verbindungsAbbau_c(to_node_id);
    
    cob_id = this->verbindungsAufbau_c(to_node_id);
    leseBezeichnungAusgaenge(sock_, cob_id);
    std::cout << "]}" << std::endl;

    //verbindungsAbbau(sock_, self_node_id_, to_node_id);
    this->verbindungsAbbau_c(to_node_id);

    return 0;
}

//private
int Uvr_Can::verbindungsAbbau_c(int to_node_id) {
    //ASSUMPTION: Mutex muss gesetzt sein
    // sende Nachricht zum Verbindungsabbau
    struct can_frame can_frame;
    canopen_frame_t canopen_frame;
    int bytes_read;
    unsigned int cob_id = self_node_id_;

    pdoHelper(sock_, self_node_id_, to_node_id, DEAKTIVIERUNG);

    bytes_read = read(sock_, &can_frame, sizeof(struct can_frame));
    // empfange Antwort
    return 0;
}

//private
int Uvr_Can::verbindungsAufbau_c(int to_node_id) {
    //ASSUMPTION: Mutex muss gesetzt sein

    struct can_frame can_frame;
    canopen_frame_t canopen_frame;
    int bytes_read;
    unsigned int cob_id = self_node_id_;

    // sende Nachricht zum Verbindungsaufbau   
    pdoHelper(sock_, self_node_id_, to_node_id, AKTIVIERUNG);

    // empfange Antwort
    bytes_read = read(sock_, &can_frame, sizeof(struct can_frame));

    if (bytes_read < 0) {
            perror("read: can raw socket read"); // perror is C old style
    }

    if (bytes_read < (int)sizeof(struct can_frame)) {
            std::cerr << "read: incomplete CAN frame" << std::endl;
    }

    if (canopen_frame_parse(&canopen_frame, &can_frame) != 0) {
            std::cerr << "CANopen failed to parse frame" << std::endl;
    } else {
        // Antwort war erfolgreich
        if(canopen_frame.data_len >=5 ){
                cob_id = (unsigned int)(canopen_frame.payload.data[4]);
        }
    }

    return cob_id;
}

int mycanopen_sdo_upload_exp(int sock, uint8_t node, uint16_t index, uint8_t subindex, uint32_t *data) {
    int retry = 0, max_retries = 3;
    int ret = 1; // 0 means success

    while((ret == 1) && (retry < max_retries)) {
        ret = canopen_sdo_upload_exp(sock, node, index, subindex, data);
       
        if(ret == 1) {
            //wait
            usleep(WAITONERROR);
        }
        retry++;
    }

    return ret;
}

//public
int Uvr_Can::isUVR_c(int to_node_id) { 

    boost::mutex::scoped_lock lock(can_if_mtx_);

    uint32_t vendor = 0, product = 0, version = 0, identifier = 0;
    int ret = 0, cob_id = 0;

    //cob_id = verbindungsAufbau(sock_, self_node_id_, to_node_id);
    cob_id = this->verbindungsAufbau_c(to_node_id);

    mycanopen_sdo_upload_exp(sock_, cob_id, 0x1018, 0x01, &vendor);
    mycanopen_sdo_upload_exp(sock_, cob_id, 0x1018, 0x02, &product);
    mycanopen_sdo_upload_exp(sock_, cob_id, 0x1018, 0x03, &version);

    mycanopen_sdo_upload_exp(sock_, cob_id, 0x23e2, 0x01, &identifier);

    verbindungsAbbau(sock_, self_node_id_, to_node_id);

    if(vendor == 0x000000CB && product == 0x0000100B && identifier == 128) ret = 1611;

    return ret;
}

//public
int Uvr_Can::scanBus(int aktiveknoten[]) {

    struct sockaddr_can addr;
    struct ifreq ifr;
    struct timeval tv;

    int knoten;
    //int aktiveknoten[array_size] = {0};
    int self_node_id = 44;

    time_t programmstart;
    time_t jetzt;

    programmstart = time(NULL);
    jetzt = time(NULL);

    while ((jetzt - 30) < programmstart)
    {
        struct can_frame can_frame;
        canopen_frame_t canopen_frame;
        int bytes_read;

        jetzt = time(NULL);

        {   // scope for scoped_lock
            boost::mutex::scoped_lock lock(can_if_mtx_);
            bytes_read = read(sock_, &can_frame, sizeof(struct can_frame));
        }

        if (bytes_read < 0)
        {
            std::cerr << "read: can raw socket read" << std::endl;
        }

        if (bytes_read < (int)sizeof(struct can_frame))
        {
            std::cerr <<  "read: incomplete CAN frame" << std::endl;
        }

        if (canopen_frame_parse(&canopen_frame, &can_frame) != 0)
        {
            std::cerr << "CANopen failed to parse frame" << std::endl;
        }

        knoten = isNMTnodeguarding(&canopen_frame);
        //std::cout << "NMT Nachricht von: " << knoten << std::endl;

        switch(knoten) {
            case 0:
            case -1:
                break;
            default:
                //alle anderen knoten
                std::cerr << "NMT Nachricht von: " << knoten << std::endl;
                if(knoten < array_size && aktiveknoten[knoten] == 0) {
                    aktiveknoten[knoten] = 1;
                }
                if(knoten < array_size && aktiveknoten[knoten] != 1611) {
                    aktiveknoten[knoten] = this->isUVR_c(knoten);
                }
        }
    } // Ende der Suchschleife

    // TODO
    return 44;
}

//public
int Uvr_Can::heartBeatThreadFunc(int amount){

    boost::posix_time::seconds workTime(9);

    for(int t = 0; t < amount; t++) {
        if(amount == 0) {
	    t = 0;
        }

        this->heartBeatFunc();
        boost::this_thread::sleep(workTime);
    }
    return amount;
}

//public
int Uvr_Can::heartBeatFunc() {

    canopen_frame_t *frame;

    frame = canopen_frame_new();
    assert(frame != NULL);

    canopen_frame_set_nmt_ng(frame, self_node_id_);
    frame->function_code = CANOPEN_FC_NMT_NG;
    frame->rtr = 0; //CANOPEN_FLAG_RTR;

    frame->data_len = 1;
    frame->payload.data[0] = CANOPEN_NMT_NG_STATE_OP;

    boost::mutex::scoped_lock lock(can_if_mtx_);

    canopen_frame_send(sock_, frame);
    canopen_frame_free(frame);

    return 1;
}

//public
int Uvr_Can::is_initialized() {
    return ((is_initialized_ == 1) && (sock_ >= 0));
}

//public
int Uvr_Can::getSocket_c(std::string can_if) {
    
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct timeval tv= {1,0};
    boost::mutex::scoped_lock lock(can_if_mtx_);

    /* Create the socket */
    if ((sock_ = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {   
        std::cerr << "Error: Failed to create socket.\n" << std::endl;
    }

    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));

    /* Locate the interface you wish to use */
    strcpy(ifr.ifr_name, can_if.c_str());
    //can_if.copy(ifr.ifr_name, sizeof ifr.ifr_name);
    ioctl(sock_, SIOCGIFINDEX, &ifr); /* ifr.ifr_ifindex gets filled
                                      * with that device's index */
                                     // XXX add check

    /* Select that CAN interface, and bind the socket to it. */
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(sock_, (struct sockaddr*)&addr, sizeof(addr)); // XXX Add check

    is_initialized_ = 1;
    return (sock_ >= 0);
}


void getSocket(std::string can_if, int *sock){
    
    struct sockaddr_can addr;
    struct ifreq ifr;

    /* Create the socket */
    if ((*sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {   
        std::cerr << "Error: Failed to create socket.\n" << std::endl;
        //return -1;
    }

    /* Locate the interface you wish to use */
    strcpy(ifr.ifr_name, can_if.c_str());
    //can_if.copy(ifr.ifr_name, sizeof ifr.ifr_name);
    ioctl(*sock, SIOCGIFINDEX, &ifr); /* ifr.ifr_ifindex gets filled
                                      * with that device's index */
                                     // XXX add check

    /* Select that CAN interface, and bind the socket to it. */
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(*sock, (struct sockaddr*)&addr, sizeof(addr)); // XXX Add check
}


int leseWerteUVRThreadFunc(int sock, int from_node_id, int akt_knoten[]){
        int cob_id = 0;
        boost::posix_time::seconds waitTime(60);

        while(1) {
        for(int i = 0; i < array_size; i++) {
                if(akt_knoten[i] == 1611) {
                        cob_id = verbindungsAufbau(sock, from_node_id, i);
        
                        // hier erfolgt die Arbeit
                        holeZeit(sock, cob_id);
                        leseAusgaenge(sock, cob_id);
                        leseWerte(sock, cob_id);
                        leseMeldung(sock, cob_id);

                        verbindungsAbbau(sock, from_node_id, i);
                }
        }
        boost::this_thread::sleep(waitTime);
        }

        return 0;
}

void canHeartBeatFunc(int sock, int node_id)
{
    canopen_frame_t *frame;

    frame = canopen_frame_new();
    assert(frame != NULL);

    canopen_frame_set_nmt_ng(frame, node_id);
    frame->function_code = CANOPEN_FC_NMT_NG;
    frame->rtr = 0; //CANOPEN_FLAG_RTR;

    frame->data_len = 1;
    frame->payload.data[0] = CANOPEN_NMT_NG_STATE_OP;

    boost::posix_time::seconds workTime(9);

    //std::cout << "Worker: starting NMT heartbeat with node_id=" << node_id << std::endl;

    for(int t = 0; t < 5; t++) {
	//mtx_.lock();
        canopen_frame_send(sock, frame);
	//mtx_.unlock();
        boost::this_thread::sleep(workTime);
    }

    //freeing the memory of this frame
    canopen_frame_free(frame);
    //std::cout << "Worker: NMT heartbeat finished" << std::endl;
}

int isNMTnodeguarding(canopen_frame_t *frame) // rename to analyze
{
    static int block_mode[256], bm_init = 0;
    int i, ret = 0;

    if (frame == NULL) {
        return -1;
    }

    if (bm_init == 0) {
        bzero((void *)block_mode, sizeof(block_mode));
        bm_init = 1;
    }

    // NMT protocol
    switch (frame->function_code) {
        // Network ManagemenT frame: Node Guarding
        case CANOPEN_FC_NMT_NG:

            if (frame->data_len == 0 && frame->rtr) {
                break;
            }

            switch (frame->payload.nmt_ng.state & CANOPEN_NMT_NG_STATE_MASK) {
                case CANOPEN_NMT_NG_STATE_BOOTUP:
                case CANOPEN_NMT_NG_STATE_DISCON:
                case CANOPEN_NMT_NG_STATE_CON:
                case CANOPEN_NMT_NG_STATE_PREP:
                case CANOPEN_NMT_NG_STATE_STOP:
                case CANOPEN_NMT_NG_STATE_OP:
                case CANOPEN_NMT_NG_STATE_PREOP:
                    ret = frame->id;
                    break;
                default:
                    ;
            }
            break;

        default:
            ;
    }

    return ret;
}


void pdoHelper(int sock, int from_node_id, int to_node_id, uint8_t action) {

    canopen_frame_t *frame;

    frame = canopen_frame_new();
    assert(frame != NULL);

    canopen_frame_set_pdo_request(frame, from_node_id);

    // looks like we need to overwrite some presets
    frame->function_code = CANOPEN_FC_PDO3_RX; //0x08 << 7 = 0x400
    frame->rtr = 0;

    frame->data_len = 8;
    frame->payload.data[0] = 0x80 | (to_node_id & 0x7F); //SDO request with server node_id
    frame->payload.data[1] = action; //00
    frame->payload.data[2] = 0x1f; //1F
    frame->payload.data[3] = 0x00; //subindex 0x00
    frame->payload.data[4] = 0x00 | (to_node_id & 0x7F); //server
    frame->payload.data[5] = 0x00 | (from_node_id & 0x7F); //client
    frame->payload.data[6] = 0x80;
    frame->payload.data[7] = 0x12;

    canopen_frame_send(sock, frame);
    canopen_frame_free(frame);
}

int verbindungsAufbau(int sock, int from_node_id, int to_node_id) {
    struct can_frame can_frame;
    canopen_frame_t canopen_frame;
    int bytes_read;
    unsigned int cob_id = from_node_id;

    // sende Nachricht zum Verbindungsaufbau
    pdoHelper(sock, from_node_id, to_node_id, AKTIVIERUNG);

    // empfange Antwort
    bytes_read = read(sock, &can_frame, sizeof(struct can_frame));

    if (bytes_read < 0) {
            perror("read: can raw socket read"); // perror is C old style
    }

    if (bytes_read < (int)sizeof(struct can_frame)) {
            std::cerr << "read: incomplete CAN frame" << std::endl;
    }

    if (canopen_frame_parse(&canopen_frame, &can_frame) != 0) {
            std::cerr << "CANopen failed to parse frame" << std::endl;
    } else {
        // Antwort war erfolgreich
        if(canopen_frame.data_len >=5 ){
                cob_id = (unsigned int)(canopen_frame.payload.data[4]);
        }
    }

    return cob_id;
}

int verbindungsAbbau(int sock, int from_node_id, int to_node_id) {
    // sende Nachricht zum Verbindungsabbau
    struct can_frame can_frame;
    canopen_frame_t canopen_frame;
    int bytes_read;
    unsigned int cob_id = from_node_id;

    pdoHelper(sock, from_node_id, to_node_id, DEAKTIVIERUNG);

    bytes_read = read(sock, &can_frame, sizeof(struct can_frame));
    // empfange Antwort
    return 0;
}


int isUVR(int sock, int from_node_id, int to_node_id) {
    uint32_t vendor = 0, product = 0, version = 0, identifier = 0;
    int ret = 0, cob_id = 0;

    cob_id = verbindungsAufbau(sock, from_node_id, to_node_id);

    canopen_sdo_upload_exp(sock, cob_id, 0x1018, 0x01, &vendor);
    canopen_sdo_upload_exp(sock, cob_id, 0x1018, 0x02, &product);
    canopen_sdo_upload_exp(sock, cob_id, 0x1018, 0x03, &version);

    canopen_sdo_upload_exp(sock, cob_id, 0x23e2, 0x01, &identifier);
    //std::cout << "identifier: " << identifier << std::endl;

    verbindungsAbbau(sock, from_node_id, to_node_id);

    if(vendor == 0x000000CB && product == 0x0000100B && identifier == 128) ret = 1611;

    return ret;
}

void selektiereUVRs(int sock, int from_node_id, int akt_knoten[])
{   
    //std::cout << "Selektiere UVRs" << std::endl;

    for(int i = 0; i < array_size; i++) {
       if(akt_knoten[i] == 1) {   
          akt_knoten[i] = isUVR(sock, from_node_id, i);
       }
    }

    for(int i = 0; i < array_size; i++) {
        if(akt_knoten[i] == 1611) {
                //std::cout << "---UVR Knoten: " << i << std::endl;
		;
        }
    }
}


int can_SDO_up_seg_wrapper(int sock, uint8_t node, u_int16_t index, u_int8_t subindex, u_int8_t *array, size_t arraysize) {
	int len = 0;
	int retry = 0;

	do {
		len = 0;
		len = canopen_sdo_upload_seg(sock, node, index, subindex, array, arraysize);

		if(len >= 0) {
			//raus hier
			;
		} else {
			// nochmal
                        usleep(WAITONERROR);
			retry++;
		}
	} while(len <= 0 && retry < RETRY); 

	return len;
}

int printBin(uint8_t arr[], int len)
{
	int i = 0;
	for (i = 0; i < len; i++)
		printf("%.2x ", arr[i]);
	printf("\n");

	if (len > 2) {
		for (i = 0; i < 8; i++)
			printf("A%d: %d\n", i + 1, CHECK_BIT(arr[0], i));
		for (i = 0; i < 8; i++)
			printf("A%d: %d\n", i + 9, CHECK_BIT(arr[1], i));
	}

	return 0;
}

int leseAusgaenge(int sock, uint16_t node)
{
	int i = 0;
	int j = 0;
	int len = 0;

	uint8_t ein[256], aus[256];

	int error = 0;

	bzero((void *)ein, sizeof(ein));
	bzero((void *)aus, sizeof(aus));

	int ersteiteration = 1;

	uint16_t index = 0x20d1;
	uint8_t subindex = 1;

	if ((len = can_SDO_up_seg_wrapper(sock, node, index, subindex, ein,
				    sizeof(ein))) >= 0) {
		;
	} else {
		printf
		    ("leseAusgaenge (1) ERROR: segmented SDO upload failed\n");
		error = 1;
	}

	if (!error) {
		index = 0x20d0;

		if ((len = can_SDO_up_seg_wrapper(sock, node, index, subindex, aus,
					    sizeof(aus))) >= 0) {
			;
		} else {
			printf
			    ("leseAusgaenge ERROR (II) : segmented SDO upload failed\n");
			error = 1;
		}
	}

	if (len > 2) {
		for (j = 0; j < 2; j++) {
			for (i = 0; i < 8; i++) {
				if (CHECK_BIT(aus[j], i) == 0
				    && CHECK_BIT(ein[j], i) == 0) {
					;
				} else {
					//printf("A%d: %d\n", i+1+j*8, CHECK_BIT(ein[j], i));
					if (ersteiteration) {
						;	// mach nichts
						ersteiteration = 0;
					} else {
						printf(",");
					}

					printf("{\n\t\"nr\": %d,\n\t\"wert\": %s\n}\n",
					       i + 1 + j * 8, CHECK_BIT(ein[j],i) ? "1" : "0");

				}
			}
		}
	}
	return error;
}

int inKommaZahl(uint8_t arr[], int len, float *wert, char *einheit)
{
	int i = 0;
	std::string einheiten[] =
	    { "unused", "C", "W/qm", "l/h", "5", "6", "7", "8", "%" };
	float t;
	int ret = 1;

	if (len >= 6) {
		int temp2 = (0x0f & arr[1 + i]) * 256 + (arr[0 + i] & 0xff);
		if (arr[1] & UVR1611) {
			temp2 = temp2 ^ 0xfff;
			temp2 = -temp2 - 1;
		}

		if (arr[4] == 65) {
			t = temp2 / 10.;
		} else {
			t = temp2;
		}

		// wenn das feld unused anzeigt
		if (arr[5] != 0) {
			if (DEBUG) {
				printf(" %.1f %s A0 %d typ %x \n", (float)t,
				       einheiten[arr[5]].c_str(), arr[4],
				       arr[6] & 0x70);
			}

			*wert = t;
			strcpy(einheit, einheiten[arr[5]].c_str());

			ret = 0;
		}
	}
	return ret;
}

int auswertungString(int sock, uint8_t node, uint16_t index, uint16_t subindex,
		     std::string &text)
{
	uint8_t data_array[256];
	int len = 0;
	int i = 0;
	int error = 0;

	bzero((void *)data_array, sizeof(data_array));

	if (DEBUG)
		printf("index: %d subindex: %d\n", index, subindex);

	if ((len =
	     can_SDO_up_seg_wrapper(sock, node, index, subindex, data_array,
				    sizeof(data_array))) >= 0) {
		if (DEBUG)
			printf("len: %d", len);

		for (i = 0; i < len; i++) {
			if (data_array[i] >= 32 && data_array[i] < 128) {
				//printf("%c", data_array[i]);
				text.append(boost::to_string(data_array[i]));
			} else {
				text.append(boost::to_string('_'));
			}
		}

		if (DEBUG) {
			printf("\n\n");
			for (i = 0; i < len; i++) {
				printf("%x", data_array[i]);
			}
		}
	} else {
		return 0;
	}
	return len;
}

int holeZeit(int sock, uint16_t node)
{
	uint16_t tag = 0x2014;
	uint16_t monat = 0x2015;
	uint16_t jahr = 0x2016;

	uint16_t stunden = 0x2012;
	uint16_t minuten = 0x2011;
	// subindex ist hier 1
	uint8_t subindex = 0x01;

	uint8_t data_array[256];
	int len = 0;
	int error = 1;
	char *t;

	printf("\"uvrzeit\": ");

	bzero((void *)data_array, sizeof(data_array));
	if ((len = can_SDO_up_seg_wrapper(sock, node, jahr, subindex, data_array,
				    sizeof(data_array))) >= 0) {
		printf("\"20%d-", data_array[0]);
		;
	} else {
		error = 0;
		printf("jahr");
	}

	bzero((void *)data_array, sizeof(data_array));
	if ((len = can_SDO_up_seg_wrapper(sock, node, monat, subindex, data_array,
				    sizeof(data_array))) >= 0) {
		printf("%02d-", data_array[0]);
		;
	} else {
		error = 0;
		printf("monat");
	}

	bzero((void *)data_array, sizeof(data_array));
	if ((len = can_SDO_up_seg_wrapper(sock, node, tag, subindex, data_array,
				    sizeof(data_array))) >= 0) {
		printf("%02dT", data_array[0]);
		;
	} else {
		error = 0;
		printf("tag");
	}

	bzero((void *)data_array, sizeof(data_array));
	if ((len = can_SDO_up_seg_wrapper(sock, node, stunden, subindex, data_array,
				    sizeof(data_array))) >= 0) {
		printf("%02d:", data_array[0]);
		;
	} else {
		error = 0;
		printf("stunden");
	}

	// TODO Zulu Zeit ist hier nur behauptet aber nicht ermittelt
	bzero((void *)data_array, sizeof(data_array));
	if ((len = can_SDO_up_seg_wrapper(sock, node, minuten, subindex, data_array,
				    sizeof(data_array))) >= 0) {
		printf("%02d:00+01:00\"", data_array[0]);
		;
	} else {
		error = 0;
		printf("minuten");
	}

	//printf("\n},");

	if (error == 0) {
		printf("holeZeit ERROR: segmented SDO upload failed\n");

	}

	return error;
}

int leseMeldung(int sock, uint16_t node)
{
	uint8_t data_array[256];
	std::string text;
	int len = 0;
	int i = 0;
	int error = 0;
	int res = 0;

	float r = 0.;
	char einheit[32];

	uint16_t index = 0x2400;	// Ausgaenge haben index 20A5
	uint16_t subindex = 0x01;

	if (DEBUG) {
		printf("Lese Meldung\n");
	}

	bzero((void *)data_array, sizeof(data_array));
	//bzero((void *)text, sizeof(text));

	if ((len = can_SDO_up_seg_wrapper(sock, node, index, subindex, data_array,
				    sizeof(data_array))) >= 0) {

		if (len >= 7) {
			if (data_array[6] & 0x40)
				inKommaZahl(data_array, len, &r, einheit);

			if (data_array[6] & 0x10) {
				if (DEBUG)
					printf("da5: %x da4: %x\n",
					       data_array[5], data_array[4]);
				if ((res =
				     auswertungString(sock, node,
						      (uint16_t) (data_array[5]
								  * 256 +
								  data_array
								  [4]),
						      (uint16_t) data_array[0],
						      text)) != 0) {
					//text[res] = '\0';
					//printf("\"%s\"", text);
					std::cout << text << std::endl;
				}
			}
		}
	} else {
		printf("leseMeldung ERROR: segmented SDO upload failed\n");
		error = 0;
	}

	return error;
}

int leseNetzwerkEingaenge(int sock, uint16_t node)
{
	uint8_t data_array[256];
	std::string text;
	int len = 0;
	int i = 0;
	int error = 0;
	int res = 0;

	float r = 0.;
	char einheit[32];

	uint16_t index = 0x220B;	// Ausgaenge haben index 20A5
	uint16_t subindex = 0x11;

	if (DEBUG) {
		printf("Lese Netzwerkeingaenge\n");
	}

	bzero((void *)data_array, sizeof(data_array));
	//bzero((void *)text, sizeof(text));

	for (i = subindex; i < subindex + 16; i++) {

		if ((len = can_SDO_up_seg_wrapper(sock, node, index, i, data_array,
					    sizeof(data_array))) >= 0) {

			if (len >= 7) {
				if (data_array[6] & 0x40) {
					res =
					    inKommaZahl(data_array, len, &r,
							einheit);
					if (res == 0)
						printf("%d: %d: %.1f %s\n", res,
						       i, r, einheit);
				}
				if (data_array[6] & 0x10) {
					if (DEBUG)
						printf("da5: %x da4: %x\n",
						       data_array[5],
						       data_array[4]);
					if (res =
					    auswertungString(sock, node,
							     (uint16_t)
							     (data_array[5] *
							      256 +
							      data_array[4]),
							     (uint16_t)
							     data_array[0],
							     text) != 0) {
						//text[res] = '\0';
						//printf("%s", text);
						std::cout << text << std::endl;
					}
				}
			}
		} else {
			printf
			    ("leseNetzwerEingaenge ERROR: segmented SDO upload failed\n");
			error = 1;
		}
	}
	return error;
}

int leseBezeichnungEA(int sock, uint16_t node, uint16_t index, int range,
		      const std::string &typ)
{
	uint8_t data_array[256];
	std::string text = "";
	int len = 0;
	int i = 0;
	int error = 0;

	int res = 0;
	int erstiteration = 1;

	//uint16_t index = 0x2084; 
	uint16_t subindex = 0x01;

	for (i = subindex; i < subindex + range; i++) {
		bzero((void *)data_array, sizeof(data_array));
		//bzero((void *)text, sizeof(text));
                text = "";

		if ((len = can_SDO_up_seg_wrapper(sock, node, index, i, data_array,
					    sizeof(data_array))) >= 0) {
			if (data_array[6] & 0x10) {
				if (DEBUG) {
					printf("da5: %x da4: %x\n",
					       data_array[5], data_array[4]);
				}
				if ((res =
				     auswertungString(sock, node,
						      (uint16_t) (data_array[5]
								  * 256 +
								  data_array
								  [4]),
						      (uint16_t) data_array[0],
						      text)) != 0) {
					//text[res] = '\0';
					if (text.compare(0,5, "  ---") != 0) {
						if (!erstiteration) {
							printf(",");
						}
						erstiteration = 0;

						char buff[10];
						bzero((void *) buff, sizeof(buff));
						int knoten = node;
						snprintf(buff, sizeof(buff), "%d_%d_%c%d", knoten, 1, typ[0], i);
  
						printf("\n{\n\t\"%s\" : %d,\n\t\"bezeichnung\": \"%s\",\n\t\"einheit\": \"%s\"\n}", typ.c_str(), i, text.c_str(), "unbekannt");
                                                //printf("\n{\n\t\"%s\" : %s\n}", buff, text.c_str());
					}
				}
			}
		} else {
			error = 1;
		}
	}
}

int leseBezeichnungEingaenge(int sock, uint16_t node)
{
	return leseBezeichnungEA(sock, node, (uint16_t) 0x2084, 16, std::string("eingang"));
}

int leseBezeichnungAusgaenge(int sock, uint16_t node)
{
	return leseBezeichnungEA(sock, node, (uint16_t) 0x20A5, 14, std::string("ausgang"));
}

int leseWerte(int sock, uint16_t node)
{

	uint8_t data_array[256];
	bzero((void *)data_array, sizeof(data_array));

	std::string text;
	//bzero((void *)text, sizeof(text));

	int len = 0;
	int res = 0;
	int i = 0;

	const uint16_t index_rahmen1 = 0x208d;
	const uint16_t index_rahmen2 = 0x220b;
	const uint8_t subindex_rahmen1 = 0x01;
	const uint8_t subindex_rahmen2 = 0x11;
	int error = 1;

	uint16_t index[32];
	uint8_t subindex[32];

	int erstiteration = 1;

	for (i = 0; i < 16; i++) {
		index[i] = index_rahmen1;
		subindex[i] = subindex_rahmen1 + i;
	}

	for (i = 0; i < 16; i++) {
		index[i + 16] = index_rahmen2;
		subindex[i + 16] = subindex_rahmen2 + i;
	}

	float r = 0.;
	char einheit[32];

	for (i = 0; i < 32; i++) {

		if ((len = can_SDO_up_seg_wrapper(sock, node, index[i], subindex[i],
					    data_array,
					    sizeof(data_array))) >= 0) {
			if (len >= 7) {
				if (data_array[6] & 0x40) {
					res =
					    inKommaZahl(data_array, len, &r,
							einheit);
					if (res == 0) {
						if (!erstiteration) {
							printf(",");
						}
						erstiteration = 0;
						printf
						    ("\n{\n\t\"nr\" : %d,\n\t\"wert\": %f,\n\t\"einheit\": \"%s\"\n}",
						     (i+1), r, einheit);
					}
				}
				if (data_array[6] & 0x10) {
					if (DEBUG)
						printf("da5: %x da4: %x\n",
						       data_array[5],
						       data_array[4]);
					if ((res =
					     auswertungString(sock, node,
							      (uint16_t)
							      (data_array[5] *
							       256 +
							       data_array[4]),
							      (uint16_t)
							      data_array[0],
							      text)) != 0) {
						text[res] = '\0';

						if (text.compare(0,5, "  ---") !=
						    0) {
							int boolwert = 1;
							if (text.compare(0,4, " AUS") == 0) {
								boolwert = 0;
							}
							if (!erstiteration) {
								printf(",");
							}
							erstiteration = 0;

							printf
							    ("\n{\n\t\"nr\" : %d,\n\t\"wert\": %d,\n\t\"einheit\": \"%s\"\n}",
							     i+1, boolwert,
							     "bool");
						}
					}
				}
			}

			bzero((void *)data_array, sizeof(data_array));

		} else {
			printf("----ERROR: segmented SDO upload failed\n");
			error = 0;
		}
	}
}
