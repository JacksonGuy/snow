#include <iostream>
#include <cassert>

#include "core/utils.hpp"
#include "net/server.hpp"

int main(int argc, char* argv[]) {
    using namespace snow;
    Server server(8000);

    server.init();
    server.start([](Server& server) {

    });

    return 0;
}
