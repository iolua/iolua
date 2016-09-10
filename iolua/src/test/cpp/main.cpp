//
// Created by liyang on 15/11/13.
//

#include <lemon/log/log.hpp>
#include <lemon/test/test.hpp>

using namespace lemon::test;

int main() {
	lemon::log::add_sink(std::unique_ptr<lemon::log::sink>(new lemon::log::console()));

    runner::run();

    lemon::log::close();
}