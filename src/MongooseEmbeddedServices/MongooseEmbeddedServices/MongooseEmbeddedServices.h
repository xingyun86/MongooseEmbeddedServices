// MongooseEmbeddedServices.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <thread>
#include <string>
// TODO: Reference additional headers your program requires here.
#include <mongoose.h>

class MongooseEmbeddedServices {
#define MES MongooseEmbeddedServices::Inst()
public:
    std::thread m_thread_server;
    sig_atomic_t s_signal_received = 0;
    const char* s_http_port = "8000";
    struct mg_serve_http_opts s_http_server_opts;

    static void signal_handler(int sig_num) {
        signal(sig_num, signal_handler);  // Reinstantiate signal handler
        MES->s_signal_received = sig_num;
    }

    int is_websocket(const struct mg_connection* nc) {
        return nc->flags & MG_F_IS_WEBSOCKET;
    }

    void broadcast(struct mg_connection* nc, const struct mg_str msg) {
        struct mg_connection* c;
        char buf[500];
        char addr[32];
        mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
            MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

        snprintf(buf, sizeof(buf), "%s %.*s", addr, (int)msg.len, msg.p);
        printf("%s\n", buf); /* Local echo. */
        for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
            if (c == nc) continue; /* Don't send to the sender. */
            mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, buf, strlen(buf));
        }
    }

    static void ev_handler(struct mg_connection* nc, int ev, void* ev_data MG_UD_ARG(void* user_data)) {
        switch (ev) {
        case MG_EV_TIMER: {
            double now = *(double*)ev_data;
            double next = mg_set_timer(nc, 0) + 1.0;
            printf("timer event, current time: %.2lf\n", now);
            /* New websocket message. Tell everybody. */
            std::string t = std::to_string(time(0));
            struct mg_str d = { t.data(), t.size() };
            MES->broadcast(nc, d);
            mg_set_timer(nc, next);  // Send us timer event again after 0.5 seconds
            break;
        }
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
            /* New websocket connection. Tell everybody. */
            MES->broadcast(nc, mg_mk_str("++ joined"));
            break;
        }
        case MG_EV_WEBSOCKET_FRAME: {
            struct websocket_message* wm = (struct websocket_message*)ev_data;
            /* New websocket message. Tell everybody. */
            struct mg_str d = { (char*)wm->data, wm->size };
            MES->broadcast(nc, d);
            break;
        }
        case MG_EV_HTTP_REQUEST: {
            mg_serve_http(nc, (struct http_message*)ev_data, MES->s_http_server_opts);
            break;
        }
        case MG_EV_CLOSE: {
            /* Disconnect. Tell everybody. */
            if (MES->is_websocket(nc)) {
                MES->broadcast(nc, mg_mk_str("-- left"));
            }
            break;
        }
        }
    }

    int run() {
        struct mg_mgr mgr;
        struct mg_connection* nc;

        signal(SIGTERM, signal_handler);
        signal(SIGINT, signal_handler);
        //setvbuf(stdout, NULL, _IOLBF, 0);
        //setvbuf(stderr, NULL, _IOLBF, 0);

        mg_mgr_init(&mgr, NULL);

        nc = mg_bind(&mgr, s_http_port, ev_handler MG_UD_ARG(NULL));
        mg_set_protocol_http_websocket(nc);
        s_http_server_opts.document_root = ".";  // Serve current directory
        s_http_server_opts.enable_directory_listing = "yes";  
        
        // Send us MG_EV_TIMER event after 2.5 seconds
        mg_set_timer(nc, mg_time() + 1);

        printf("Started on port %s\n", s_http_port);
        while (s_signal_received == 0) {
            mg_mgr_poll(&mgr, 1000);
        }
        mg_mgr_free(&mgr);

        return 0;
    }
    int start_service()
    {
        m_thread_server = std::thread([]() {
            MES->run();
            });
        return 0;
    }
    int close_service()
    {
        s_signal_received = (!0);
        if (m_thread_server.joinable())
        {
            m_thread_server.join();
        }
        return 0;
    }
public:
    static MongooseEmbeddedServices* Inst() {
        static MongooseEmbeddedServices mongooseEmbeddedServicesInstance;
        return &mongooseEmbeddedServicesInstance;
    }
};