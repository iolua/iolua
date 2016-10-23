//
// Created by liyang on 15/11/13.
//
#include <iostream>
#include <iolua/iolua.hpp>


int main(int args, char** argv) {

    if(args < 2) {
        return 0;
    }

    iolua::iolua_State state;

    std::vector<std::string> argList;

//    lemon::log::add_sink(std::unique_ptr<lemon::log::sink>(new lemon::log::console()));

    for( int i = 2; i < args; i ++ ) {
        argList.push_back(argv[i]);
    }

    try {
        state.start(argv[1],argList);

        state.join();

    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
        state.exit();
    }

	lemon::log::close();

}