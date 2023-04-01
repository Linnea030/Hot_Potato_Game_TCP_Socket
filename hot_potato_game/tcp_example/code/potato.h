#include <string.h>
#include <iostream>
#include <vector>

class Potato{
public:
    //int nums;
    int hops;
    int total_hops;
    //std::vector<int> track;
    int track[512];
    bool isClose;

    Potato() {
        //nums = 0;
        isClose = false;
        hops = -1;
        memset(track, 0, sizeof(track));
        //track.clear();
       //track.resize(512,0);
    }

    Potato(bool i) {
        isClose = true;
        hops = -1;
        memset(track, 0, sizeof(track));
        //track.clear();
        //track.resize(512,0);
    }

    void print_track();
    void print_potato();
};

    void Potato::print_track() {
        std::cout<< "Trace of potato:\n";
        for(int i = 0; i < total_hops; i++) {
            std::cout<<track[i];
            if(i != total_hops - 1){
                std::cout<<",";
            }
        }
        std::cout<<"\n";
    }

    void Potato::print_potato() {
        std::cout<< "\n";
        std::cout<< "info of potato:\n"<< "hops of potato: "<<hops<<"\n"<< "total_hops of potato: "<<total_hops<<"\n";
    }