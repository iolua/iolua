//
// Created by liyang on 15/11/13.
//
#include <iostream>
#include <lemon/log/log.hpp>
#include <iolua/iolua.hpp>


int main(int args, char** argv) {

    if(args < 2) {
        return 0;
    }

    lemon::log::add_sink(std::unique_ptr<lemon::log::sink>(new lemon::log::console({ "console" })));

    iolua::iolua_State state;

    std::vector<std::string> argList;

    for( int i = 2; i < args; i ++ ) {
        argList.push_back(argv[i]);
    }

    try {
        state.start(argv[1],argList);

        state.join();

    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }


}