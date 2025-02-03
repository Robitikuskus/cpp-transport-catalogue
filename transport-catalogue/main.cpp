#include <iostream>
#include <fstream>

#include "json_reader.h"
#include "request_handler.h"

int main() {
    using namespace std::literals;

    TransportCatalogue catalogue;

    std::ifstream fin("tests//input.json");
    JsonReader reader(fin);

    // JsonReader reader(std::cin);
    reader.FillCatalogue(catalogue);

    MapRenderer renderer(reader.GetRenderSettings());
    TransportRouter router(catalogue, reader.GetRoutingSettings());

    RequestHandler handler(catalogue, renderer, router);

    std::ofstream fout("tests//output.json");
    handler.PrintRequestsResponce(reader.GetStatRequests(), fout);

    // handler.PrintRequestsResponce(reader.GetStatRequests(), std::cout);
}