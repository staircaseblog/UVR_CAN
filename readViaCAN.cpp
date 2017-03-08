// License: Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported (CC BY-NC-ND 3.0)

#include "uvr_can.hpp"

int main(int argc, char **argv)
{
    uint8_t node;
    char *zeit;

    if (argc != 3 && argc != 4) {
        std::cerr << "usage: " << argv[0] << " can-interface NODE {werte/bezeichnung}" << std::endl;
        return -1;
    }

    std::string can_if(argv[1]);

    Uvr_Can uvr;
    uvr.getSocket_c(can_if);

    node = strtol(argv[2], NULL, 16);

    //boost::thread hBT( boost::bind( &Uvr_Can::heartBeatThreadFunc, &uvr, 5) );

    //node = std::stoul(argv[2], nullptr, 16);
    int error = 0;

    if (argc == 4 && strncmp("werte", argv[3], 5) == 0) {
       uvr.leseWerte_c(node);
    } else {
       uvr.leseBeschreibung_c(node);
    }

    //hBT.join();

    return error;
}
