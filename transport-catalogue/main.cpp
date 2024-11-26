#include <iostream>
#include <fstream>

#include "json_reader.h"
#include "request_handler.h"

int main() {
    TransportCatalogue catalogue;

    JsonReader reader(std::cin);
    reader.FillCatalogue(catalogue);

    MapRenderer renderer(reader.GetRenderSettings());

    RequestHandler handler(catalogue, renderer);
    handler.PrintRequestsResponce(reader.GetStatRequests(), std::cout);
}