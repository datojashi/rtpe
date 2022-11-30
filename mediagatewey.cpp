#include "mediagatewey.h"


MediaGatewey::MediaGatewey(std::string remote_addr) : m_sip_addr(remote_addr)
{
}


bool MediaGatewey::process_SFU_sdp(std::string s)
{
    bool result=false;



    //std::cout << s << std::endl;

    std::vector<std::string> slist;
    opt::split(s,"m=",slist,false);

    std::cout << slist.size() << std::endl;

    for(int i=0; i<slist.size(); i++)
    {
        std::cout << slist.at(i) << std::endl;
        //std::cout << "*********************" << std::endl;
    }


}

bool MediaGatewey::createMedia()
{

    /*
     std::fstream fs;
     std::stringstream ss;
     fs.open("local_sdp.txt");
     ss << fs.rdbuf();
     local_sdp = ss.str();
    //*/


    std::ifstream fs;
    std::string s;
    fs.open("sdp_offer.txt");
    s=std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());


    std::cout << s << std::endl;

    pj_status_t status;

    MG_ASSERT( pj_init());
    MG_ASSERT(pjlib_util_init());
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

    pj_thread_desc desc;
    pj_thread_t *thr;
    MG_TRY(pj_thread_register("aaaa", desc, &thr),throwException("Cannot register thread"));
    MG_ASSERT(pjmedia_endpt_create(&cp.factory, NULL, 1, &m_med_endpt));

    pjmedia_endpt_dump(m_med_endpt);

    pool = pjmedia_endpt_create_pool(m_med_endpt, "Media pool", 4096, 4096);

    if(pool!=NULL)
    {
        std::cout << "Media pool init OK" << std::endl;
    }

#ifdef A_ENGINE
    MG_ASSERT(initAudioCodecs());
#endif
#ifdef __V_ENGINE
    MG_ASSERT(initVideoCodecs());
#endif

    MG_ASSERT(pjmedia_event_mgr_create(pool, 0, NULL));

    sdp.clear();
    opt::StringList siplist;
    opt::split(sipmsg,"\r\n\r\n",siplist);

    if(siplist.size() > 1)
    {
        sdp=siplist.at(1);
    }

    std::cout << "***********************************" << siplist.size() << std::endl;
    std::cout << sdp << std::endl;

    /*
    for(int i=0; i<sdplist.size(); i++)
    {
        if( opt::startsWith(sdplist.at(i),"o="))
        {
            std::cout << sdplist.at(i) << std::endl;
        }
    }
    */




    MG_TRY(pjmedia_conf_create(pool,32,CLOCK_RATE,NCHANNELS,NSAMPLES,NBITS,0,&conf),throwException("Cannot create conference bridje"));

    status = pjmedia_wav_writer_port_create(  pool, "confrecord.wav",
                                              CLOCK_RATE, NCHANNELS, NSAMPLES,
                                              NBITS, 0, 0,
                                              &rec_port);

    pjmedia_conf_add_port(conf, pool, rec_port, NULL, NULL);


    pjmedia_wav_player_port_create(pool,"jbsim.wav",20,0,0,&play_port);
    pjmedia_conf_add_port(conf, pool, play_port, NULL, NULL);


    //status = pjmedia_vid_conf_create(pool,NULL,&vid_conf);
    //pjmedia_session_create()


    //*
    for(int i=0;  i<MAX_STREAMS; i++)
    {
        MG_ASSERT(create_udp_transport(pj_AF_INET(), NULL, NULL, 0, i));
        pjmedia_transport_info_init(&m_med_tpinfo[i]);
        pjmedia_transport_get_info(m_med_transport[i], &m_med_tpinfo[i]);
        pj_memcpy(&m_sock_info[i], &m_med_tpinfo[i].sock_info, sizeof(pjmedia_sock_info));
        pj_uint16_t rtp_port=get_rtp_port(i);
        pj_uint16_t rtcp_port=get_rtcp_port(i);
        std::cout << "rtp_port=" << rtp_port << std::endl;
        std::cout << "rtcp_port=" << rtcp_port << std::endl;


        MG_ASSERT(create_media_stream(i,0));

    }
    //*/


    if (status != PJ_SUCCESS)
    {
        std::cout << "Error create recorder port" << std::endl;
        return false;
    }



    pjmedia_conf_connect_port(conf,2,3,0);
    pjmedia_conf_connect_port(conf,3,0,0);

    //pjmedia_conf_connect_port(conf,0,1,0);
    //pjmedia_conf_connect_port(conf,1,0,0);

    int p_ct=pjmedia_conf_get_port_count(conf);
    int c_ct=pjmedia_conf_get_connect_count(conf);

    std::cout << "************* " << p_ct << "\t" << c_ct << std::endl;

    conf_list(1);

    std::cout << "***** Media create thread ID: " << std::this_thread::get_id() << std::endl;
    return true;
}


pj_status_t MediaGatewey::create_udp_transport(int af, const char *name, const pj_str_t *addr, unsigned options, int index)
{
    pj_status_t status=PJ_SUCCESS;
    pj_bzero(&si, sizeof(pjmedia_sock_info));
    si.rtp_sock = PJ_INVALID_SOCKET;
    si.rtcp_sock = PJ_INVALID_SOCKET;

    //rtp
    MG_TRY(pj_sock_socket(af, pj_SOCK_DGRAM(), 0, &si.rtp_sock), sockErrorHandler());
    MG_TRY(pj_sockaddr_init(af, &si.rtp_addr_name, addr, 1234+index), sockErrorHandler());
    MG_TRY(pj_sock_bind(si.rtp_sock, &si.rtp_addr_name, pj_sockaddr_get_len(&si.rtp_addr_name)), sockErrorHandler());
    //rtcp
    MG_TRY(pj_sock_socket(af, pj_SOCK_DGRAM(), 0, &si.rtcp_sock), sockErrorHandler());
    MG_TRY(pj_sockaddr_init(af, &si.rtcp_addr_name, addr, 1235+index), sockErrorHandler());
    MG_TRY(pj_sock_bind(si.rtcp_sock, &si.rtcp_addr_name, pj_sockaddr_get_len(&si.rtcp_addr_name)), sockErrorHandler());

    return pjmedia_transport_udp_attach(m_med_endpt, name, &si, options, &m_med_transport[index]);
}

pj_status_t MediaGatewey::create_media_stream(int index, unsigned int sdp_index)
{

    pj_status_t status=PJ_SUCCESS;


    pjmedia_sdp_session* ses;
    MG_TRY(pjmedia_sdp_parse(pool,(char*)sdp.data(), sdp.size(),&ses),nullptr);
    m_remote[index] = *ses;
    MG_TRY(pjmedia_sdp_parse(pool,(char*)local_sdp.data(), local_sdp.size(),&ses),nullptr);
    m_local[index] = *ses;

    MG_TRY(pjmedia_stream_info_from_sdp(&m_med_streaminfo[index], pool,  m_med_endpt,  &m_local[index], &m_remote[index], sdp_index),0);
    MG_TRY(pjmedia_stream_create(m_med_endpt,pool, &m_med_streaminfo[index],m_med_transport[index],nullptr, &m_med_stream[index]),nullptr);
    MG_TRY(pjmedia_stream_start(m_med_stream[index]),nullptr);
    MG_TRY(pjmedia_transport_media_start(m_med_transport[index],0,0,0,0),nullptr);
    MG_TRY(pjmedia_stream_get_port(m_med_stream[index], &m_ports[index]),nullptr);

    status=pjmedia_conf_add_port(conf,pool,m_ports[index],NULL,NULL);
    return status;
}



void MediaGatewey::sockErrorHandler()
{
    if (si.rtp_sock != PJ_INVALID_SOCKET)
        pj_sock_close(si.rtp_sock);
    if (si.rtcp_sock != PJ_INVALID_SOCKET)
        pj_sock_close(si.rtcp_sock);
    std::cout << "Sock error handler" << std::endl;
}

pj_uint16_t MediaGatewey::get_rtp_port(int index)
{
    pj_sockaddr addr;
    int len;
    pj_sock_getsockname(m_sock_info[index].rtp_sock,&addr,&len);
    return pj_ntohs(addr.ipv4.sin_port);
}

pj_uint16_t MediaGatewey::get_rtcp_port(int index)
{
    pj_sockaddr addr;
    int len;
    pj_sock_getsockname(m_sock_info[index].rtcp_sock,&addr,&len);
    return pj_ntohs(addr.ipv4.sin_port);
}

void MediaGatewey::getCodec()
{
    pj_str_t codec_id;
    pjmedia_codec_info *codec_info;
    unsigned count = 1;
    pjmedia_codec *codec;

    std::string scodec="G722";

    codec_id = pj_str((char*)scodec.data());

    // Find codec info for the specified coded ID (i.e. "pcma").
    pj_status_t status = pjmedia_codec_mgr_find_codecs_by_id( pjmedia_endpt_get_codec_mgr(m_med_endpt), &codec_id,
                                                              &count, (const pjmedia_codec_info**)&codec_info, NULL);

    std::cout << "=============== " << status << std::endl;

    // Allocate the codec.
    //status = pjmedia_codec_mgr_alloc_codec( codec_mgr, codec_info, &codec );
}

pj_status_t MediaGatewey::initAudioCodecs()
{
    pj_status_t status;
    pjmedia_codec_register_audio_codecs(m_med_endpt, NULL);
    getCodec();
#if defined(PJMEDIA_HAS_G711_CODEC) && PJMEDIA_HAS_G711_CODEC!=0
    status = pjmedia_codec_g711_init(m_med_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    status = pjmedia_endpt_dump(m_med_endpt);
    printf("Status=%d\n",status);
#endif
    getCodec();
    return status;
}

pj_status_t MediaGatewey::initVideoCodecs()
{
    pj_status_t status;

    status=pjmedia_video_format_mgr_create(pool, 64, 0, NULL);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    status = pjmedia_converter_mgr_create(pool, NULL);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    status=pjmedia_vid_codec_mgr_create(pool, NULL);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

#if defined(PJMEDIA_HAS_OPENH264_CODEC) && PJMEDIA_HAS_OPENH264_CODEC != 0
    status = pjmedia_codec_openh264_vid_init(NULL, &cp.factory);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);
#endif

#if defined(PJMEDIA_HAS_VID_TOOLBOX_CODEC) && \
    PJMEDIA_HAS_VID_TOOLBOX_CODEC != 0
    status = pjmedia_codec_vid_toolbox_init(NULL, &cp.factory);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);
#endif

#if defined(PJMEDIA_HAS_VPX_CODEC) && PJMEDIA_HAS_VPX_CODEC != 0
    status = pjmedia_codec_vpx_vid_init(NULL, &cp.factory);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);
#endif

#if defined(PJMEDIA_HAS_FFMPEG_VID_CODEC) && PJMEDIA_HAS_FFMPEG_VID_CODEC != 0
    status = pjmedia_codec_ffmpeg_vid_init(NULL, &cp.factory);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);
#endif


    return status;
}

void MediaGatewey::conf_list(int detail)
{
    enum { MAX_PORTS = 32 };
    unsigned i, count;
    pjmedia_conf_port_info info[MAX_PORTS];

    printf("Conference ports:\n");

    count = PJ_ARRAY_SIZE(info);
    pjmedia_conf_get_ports_info(conf, &count, info);

    for (i=0; i<count; ++i) {
        char txlist[4*MAX_PORTS];
        unsigned j;
        pjmedia_conf_port_info *port_info = &info[i];

        txlist[0] = '\0';
        for (j=0; j<port_info->listener_cnt; ++j)
        {
            char s[10];
            pj_ansi_sprintf(s, "#%d ", port_info->listener_slots[j]);
            pj_ansi_strcat(txlist, s);

        }

        if (txlist[0] == '\0')
        {
            txlist[0] = '-';
            txlist[1] = '\0';
        }

        if (!detail) {
            printf("Port #%02d %-25.*s  transmitting to: %s\n",
                   port_info->slot,
                   (int)port_info->name.slen,
                   port_info->name.ptr,
                   txlist);
        } else {
            unsigned tx_level, rx_level;

            pjmedia_conf_get_signal_level(conf, port_info->slot,
                                          &tx_level, &rx_level);

            printf("Port #%02d:\n"
                   "  Name                    : %.*s\n"
                   "  Sampling rate           : %d Hz\n"
                   "  Samples per frame       : %d\n"
                   "  Frame time              : %d ms\n"
                   "  Signal level adjustment : tx=%d, rx=%d\n"
                   "  Current signal level    : tx=%u, rx=%u\n"
                   "  Transmitting to ports   : %s\n\n",
                   port_info->slot,
                   (int)port_info->name.slen,
                   port_info->name.ptr,
                   port_info->clock_rate,
                   port_info->samples_per_frame,
                   port_info->samples_per_frame*1000/port_info->clock_rate,
                   port_info->tx_adj_level,
                   port_info->rx_adj_level,
                   tx_level,
                   rx_level,
                   txlist);
        }
    }
    puts("");
}


