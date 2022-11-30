#ifndef MEDIAGATEWEY_H
#define MEDIAGATEWEY_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>

#include <pjmedia.h>
#include <pjmedia-codec.h>

#include <sio_client.h>
#include <sio_message.h>
#include <sio_socket.h>

#include "misc_tools.h"


#define A_ENGINE
#define MAX_STREAMS 1

#define CLOCK_RATE	44100
#define NSAMPLES	(CLOCK_RATE * 20 / 1000)
#define NCHANNELS	1
#define NBITS		16

class MGException: public std::exception
{
public:
    const char* what()
    {
        return txt.c_str();
    }
    std::string txt;
};


class MediaGatewey
{

#define MG_ASSERT(op)   do { \
    status = op; \
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1); \
} while (0)

#define MG_TRY(op1,op2) \
    do{\
    status=op1; \
    if(status!=PJ_SUCCESS) \
    {\
    op2;\
    return status; \
}\
}while(0)

public:
    MediaGatewey(std::string remote_addr);
    bool createMedia();
    bool process_SFU_sdp(std::string s);

    pjmedia_port* m_ports[MAX_STREAMS];
    std::atomic_bool started=false;

    void connectToSip()
    {
        sio.socket()->on("INVITE", std::bind(&MediaGatewey::onSipMessage,this,std::placeholders::_1));
        sio.socket()->on("BYE", std::bind(&MediaGatewey::onSipMessage,this,std::placeholders::_1));
        sio.connect(m_sip_addr);
    }

    void onSipMessage(sio::event &ev)
    {
        std::string ev_name=ev.get_name();
        std::cout << "******************" << ev_name  <<std::endl;
        std::cout << "***** onSipMessage thread ID: " << std::this_thread::get_id() << std::endl;
        if(ev_name=="BYE")
        {

            started.store(false);
        }
        if(ev_name=="INVITE")
        {
            auto msgs = ev.get_messages();
            std::cout << msgs.size() << std::endl;
            for(int i=0; i<msgs.size(); i++)
            {
                sipmsg = msgs.at(i)->get_string();
            }
            started.store(true);
        }
    }

private:
    sio::client sio;
    std::mutex mutex;

    std::string m_sip_addr;

    pjmedia_endpt*    m_med_endpt;
    pjmedia_transport_info m_med_tpinfo[MAX_STREAMS];
    pjmedia_transport*    m_med_transport[MAX_STREAMS];
    pjmedia_sock_info     m_sock_info[MAX_STREAMS];

    pjmedia_stream*  m_med_stream[MAX_STREAMS];
    pjmedia_stream_info m_med_streaminfo[MAX_STREAMS];



    pjmedia_sdp_session m_local[MAX_STREAMS];
    pjmedia_sdp_session m_remote[MAX_STREAMS];

    pjmedia_conf* conf;
    pjmedia_vid_conf* vid_conf;

    pj_pool_t* m_pool;
    pj_caching_pool	     cp;
    pj_pool_t* pool = nullptr;

    pjmedia_sock_info si;

    std::string sipmsg;
    std::string sdp;
    std::string local_sdp = "v=0\r\n"
                            "o=dato 1972 3456 IN IP4 192.168.0.117\r\n"
                            "s=Talk\r\n"
                            "c=IN IP4 192.168.0.117\r\n"
                            "t=0 0\r\n"
                            "a=rtcp-xr:rcvr-rtt=all:10000 stat-summary=loss,dup,jitt,TTL voip-metrics\r\n"
                            "a=record:off\r\n"
                            "m=audio 1234 RTP/AVP 0 8 96\r\n"
                            "a=rtpmap:96 telephone-event/8000\r\n"
                            "a=rtcp-fb:* trr-int 5000\r\n"
                            "a=rtcp-fb:* ccm tmmbr";


    // std::string local_sdp;

    std::condition_variable cv;


    pjmedia_port *rec_port = NULL;
    pjmedia_port *play_port = NULL;

    MGException pjexc;

    void throwException(std::string es)
    {
        pjexc.txt="PJ Error: " + es;
        throw pjexc;
    }


    pj_status_t create_media_stream(int index, unsigned int sdp_index);
    pj_status_t create_udp_transport(int af, const char *name, const pj_str_t *addr, unsigned options, int index);
    void getCodec();
    pj_status_t initAudioCodecs();
    pj_status_t initVideoCodecs();
    void sockErrorHandler();
    pj_uint16_t get_rtp_port(int index);
    pj_uint16_t get_rtcp_port(int index);
    void conf_list(int detail);


};

#endif // MEDIAGATEWEY_H
