//
// Created by liyang on 15/11/13.
//
#include <iostream>
#include <lemon/log/log.hpp>
#include <iolua/ltask/scheduler.hpp>


int main(int args, char** argv) {

    if(args < 2) {
        return 0;
    }

    lemon::log::add_sink(std::unique_ptr<lemon::log::sink>(new lemon::log::console()));

    iolua::ltask::scheduler scheduler(std::thread::hardware_concurrency());

    std::vector<std::string> argList;

    for( int i = 2; i < args; i ++ ) {
        argList.push_back(argv[i]);
    }

    try {
        scheduler.start(argv[1],argList);

        scheduler.join();

    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }


}