//
// Created by lx on 1/18/22.
//
#include "webServer.h"

int main(int argc, char** argv) {
    WebServer ws;
    ws.eventListen();
    ws.eventLoop();
}
