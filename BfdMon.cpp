// Copyright (c) 2018 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.
// Written by: Rob Martin 2018

#include <eos/agent.h>
#include <eos/sdk.h>
#include <eos/bfd.h>
#include <eos/tracing.h>
#include <ctime>

static const std::string AGENT_NAME = "BfdMon";

class my_bfd_mon : public eos::agent_handler,
                   public eos::bfd_session_handler {
    public:
        eos::agent_mgr * agent_mgr;
        eos::bfd_session_mgr * bfd_session_mgr_;
        eos::tracer t;

        explicit my_bfd_mon(eos::sdk & sdk)
            : eos::agent_handler(sdk.get_agent_mgr()),
              eos::bfd_session_handler(sdk.get_bfd_session_mgr()),
              agent_mgr(sdk.get_agent_mgr()),
              bfd_session_mgr_(sdk.get_bfd_session_mgr()),
              t(AGENT_NAME.c_str()) {
            t.trace0("Constructed");
              }
        void on_initialized() {
            t.trace0("Initialized");
            eos::ip_addr_t ip1("10.0.0.2");
            eos::intf_id_t intf1("Ethernet1");
            eos::bfd_session_key_t(ip1,"default",eos::BFD_SESSION_TYPE_NORMAL,intf1);
            auto bfd_mgr_ = get_bfd_session_mgr();
           /* for (auto b_sess = bfd_mgr_->session_iter(); b_sess; ++b_sess) {
                bfd_mgr_->session_set(b_sess.bfd_session_key_t);
            }*/
            //bfd_session_mgr_->session_set(peer_key);
            status_update("Total BFD Peer/State changes",std::to_string(bfdChanges));
            //agent_mgr->status_set("BFD Changes:",std::to_string (bfdChanges));
            watch_all_bfd_sessions(true);
            
        }
        void on_bfd_session_status(eos::bfd_session_key_t const& bfdKey, eos::bfd_session_status_t operState){
            bfdChanges++;
            std::string bfdState = "NA";
            switch (operState) {
                case eos::BFD_SESSION_STATUS_UP:
                    bfdState = "UP";
                case eos::BFD_SESSION_STATUS_DOWN:
                    bfdState = "DOWN";
                case eos::BFD_SESSION_STATUS_INIT:
                    bfdState = "INITIALIZING";
                case eos::BFD_SESSION_STATUS_ADMIN_DOWN:
                    bfdState = "ADMIN DOWN";
                case eos::BFD_SESSION_STATUS_NULL:
                    bfdState = "NULL";
                default:
                    bfdState = "NA";
            }
            time_t now = time(0);
            std::string l_time_change = ctime(&now);
            t.trace5("The State of " << bfdKey.ip_addr.to_string << " is now " << bfdState);
            status_update("Total BFD Peer/State changes",std::to_string(bfdChanges));
            status_update("Last change of Peer " << bfdKey.ip_addr.to_string << " on " << bfdKey.intf.to_string,bfdState);
            status_update("Last time change for Peer " << bfdKey.ip_addr.to_string << " on " << bfdKey.intf.to_string,l_time_change); 
        }
    private:
        int bfdChanges = 0;
        void status_update(std::string s_name,std::string s_value) {
            agent_mgr->status_set(s_name,s_value);
        }
};

int main(int argc, char ** argv) {
    eos::sdk sdk(AGENT_NAME);
    my_bfd_mon agent(sdk);
    sdk.main_loop(argc,argv);
}