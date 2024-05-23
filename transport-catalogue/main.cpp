#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

using namespace std;

int main() {
    TransportCatalogue catalogue;

    JsonReader reader(cin);
    reader.FillCatalogue(catalogue);

    MapRenderer renderer(reader.GetRenderSettings());

    RequestHandler handler(catalogue, renderer);
    auto responce = handler.GetRequestsResponce(reader.GetStatRequests());
    json::Print(responce, cout);
}