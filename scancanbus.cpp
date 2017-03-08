// License: Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported (CC BY-NC-ND 3.0)

#include "uvr_can.hpp"


void knoten_in_json(int aktiveknoten[]) {
    std::cout << "{\"knoten\": [";
    int erste_iteration = 1;
 
    for(int i = 0; i < 64; i++) {
        if(aktiveknoten[i] == 1611) {
            if(erste_iteration) {
                erste_iteration = 0;
            } else {
                std::cout << ",";
            }
            
            std::cout << "{\"typ\": \"uvr1611\", \"can_id\": " << i << "}";
         }
    }
    std::cout << "]}" << std::endl;
}

int main(int argc, char **argv)
{
    int aktiveknoten[array_size] = {0};

    if (argc != 2)
    {
        std::cerr << "usage: %s can-interface\n" << argv[0] << std::endl;
        return -1;
    }

    std::string can_if(argv[1]);

    Uvr_Can uvr;
    uvr.getSocket_c(can_if);
    uvr.scanBus(aktiveknoten);

    //boost::thread canHeartBeatThread(canHeartBeatFunc, sock, self_node_id);

    uvr.selektiereUVRs_c(aktiveknoten);
    knoten_in_json(aktiveknoten);

    //canHeartBeatThread.join();
    return 0;
}
