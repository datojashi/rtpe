#include <iostream>

#include "mediagatewey.h"

using namespace std;

int main()
{

    MediaGatewey mgw(std::string("http://127.0.0.1:3000"));


    //* Test
    std::ifstream fs;
    std::string s;
    fs.open("sdp_offer.txt");
    s=std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
    mgw.process_SFU_sdp(s);

    return 0;
    //*/


    mgw.connectToSip();
    for(int i=0; i<MAX_STREAMS; i++)
    {
        mgw.m_ports[i]=nullptr;
    }



    while (mgw.started.load()==false)
    {
        usleep(1000);
    }

    try
    {
        mgw.createMedia();
    }
    catch( MGException e )
    {
       std::cout << "Error " << e.txt << std::endl;
    }

    while( mgw.m_ports[0]==nullptr || mgw.m_ports[1]==nullptr )
    {
        usleep(1000);
    }

    while(mgw.started.load())
    {
        usleep(100000);
    }

    return 0;
}
