// Copyright (c) 2018 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.
// Written by: Rob Martin 2018
// This has been tested with EOS 4.20.5F

#include <eos/agent.h>
#include <eos/sdk.h>
#include <eos/bfd.h>
#include <eos/tracing.h>
#include <ctime>
#include <vector>
#include <sstream>
#include <regex>

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
            status_update("Total BFD Peer/State changes",std::to_string(bfdChanges));
            watch_all_bfd_sessions(true);
            // Add call to function to print ot syslog
            /*
            eos::ip_addr_t ip1("10.0.0.2");
            eos::intf_id_t intf1("Ethernet1");
            eos::bfd_session_key_t(ip1,"default",eos::BFD_SESSION_TYPE_NORMAL,intf1);*/
            //auto bfd_mgr_ = get_bfd_session_mgr();
            
            
        }
        void on_agent_option(std::string const & optionName, std::string const & value) {
            std::vector<std::string> o_value;
            std::string vrf1;
            std::string oIP;
            std::string oIntf;
            std::regex IPre("(\\d{1,3}(\\.\\d{1,3}){3})");
            std::smatch IPmatch;
            std::regex Intfre("(Ethernet)(.*)\\d+",std::regex_constants::icase);
            std::regex Intfre2("(Eth)(.*)\\d+",std::regex_constants::icase);
            std::smatch Intfmatch;
            peers tmpPeer;
            o_value = split(value);
            int value_length = o_value.size();
            oIP = o_value[0];
            /*if (std::regex_match(o_value[0],IPmatch,IPre))
                oIP = o_value[0];
            */
           try {
               if (std::regex_match(o_value[1],Intfmatch,Intfre)){
                    oIntf = o_value[1];
                }
                else if(std::regex_match(o_value[1],Intfmatch,Intfre2)){
                    oIntf = _replace_string(o_value[1],"eth","Ethernet");
                }
            }
            catch (const std::regex_error& e) {
                std::cout << "regex_error caught: " << e.what() << '\n';
                if (e.code() == std::regex_constants::error_brack) {
                    std::cout << "The code was error_brack\n";
                }
            }
            eos::ip_addr_t ip1(oIP);
            eos::intf_id_t intf1(oIntf);
            tmpPeer.name = optionName;
            tmpPeer.ip = oIP;
            tmpPeer.intf = oIntf;
            tmpPeer.stat_chg = 0;
            tmpPeer.last = "N/A";
            if (o_value.size() == 3){
                vrf1 = o_value[2];
                tmpPeer.vrf = o_value[2];
            }
            else {
                vrf1 = "default";
                tmpPeer.vrf = "default";
            }
            peer_list_count++;
            auto bfd_key = eos::bfd_session_key_t(ip1,vrf1,eos::BFD_SESSION_TYPE_NORMAL,intf1);
            bfd_session_mgr_->session_set(bfd_key);
            
        }
        void on_bfd_session_status(eos::bfd_session_key_t const& bfdKey, eos::bfd_session_status_t operState){
            bfdChanges++;
            std::string bfdState = _get_status(operState);
            time_t now = time(0);
            std::string l_time_change = ctime(&now);
            std::string t_msg = "The State of " +  bfdKey.ip_addr().to_string() + " is now " + bfdState;
            t.trace5(t_msg.c_str());
            status_update("Total BFD Peer/State changes",std::to_string(bfdChanges));
            status_update("Last change of Peer " + bfdKey.ip_addr().to_string() + " on " + bfdKey.intf().to_string(),bfdState);
            status_update("Last time change for Peer " +bfdKey.ip_addr().to_string() + " on " + bfdKey.intf().to_string(),l_time_change); 
        }
    private:
        int bfdChanges = 0; //# of BFD Peer/State changes
        struct peers {
            std::string name;
            std::string ip;
            std::string intf;
            std::string vrf;
            std::string last;
            int stat_chg;
        };
        peers peer_list[64]; //Tracks all Peer structures
        int peer_list_count = 0; //total count for all Peer structures
        //Basic function to split ',' separated string into an array
        void _to_syslog(std::string sys_msg) {
            //Add in code to write to switch syslog
        }
        std::string _replace_string(std::string oldString,std::string patt,std::string nValue){
            int posi = 0;
            std::string newString;
            posi = oldString.find(patt);
            newString = oldString.substr(0,posi) + nValue;
            newString += oldString.substr(posi+patt.size(),std::string::npos);
            return newString;
        }
        std::vector<std::string> split(std::string mstring) {
            std::vector<std::string> mvalue;
            std::string token;
            std::istringstream ss(mstring);
            while (std::getline(ss,token,','))
            {
                mvalue.push_back(token);
            }
            return mvalue;
        }
        void status_update(std::string s_name,std::string s_value) {
            agent_mgr->status_set(s_name,s_value);
        }
        void _update_status(){
            for (auto b_sess = bfd_session_mgr_->session_iter();b_sess;b_sess++) {
                std::string bfdState = _get_status(bfd_session_mgr_->session_status(*b_sess));
                std::string bfdIP = b_sess->ip_addr().to_string();
                std::string bfdPeer;
                std::string bfdTime;
                int bfdCount;
                for (int i = 0; i < peer_list_count;i++) {
                    if (peer_list[i].ip == bfdIP) {
                        bfdPeer = peer_list[i].name;
                        bfdCount = peer_list[i].stat_chg;
                        bfdTime = peer_list[i].last;
                    }
                }
                status_update("[" + bfdPeer + "] Status: [" + b_sess->ip_addr().to_string() + " on " + b_sess->intf().to_string() + "]",bfdState);
                status_update("[" + bfdPeer + "] Last Status Change: [" + b_sess->ip_addr().to_string() + " on " + b_sess->intf().to_string() + "]",bfdTime);
                status_update("[" + bfdPeer + "] Status Change Total: [" + b_sess->ip_addr().to_string() + " on " + b_sess->intf().to_string() + "]",std::to_string(bfdCount));
            }
        }
        std::string  _get_status(eos::bfd_session_status_t operState) {
            switch (operState) {
                case eos::BFD_SESSION_STATUS_UP:
                    return "UP";
                case eos::BFD_SESSION_STATUS_DOWN:
                    return "DOWN";
                case eos::BFD_SESSION_STATUS_INIT:
                    return "INITIALIZING";
                case eos::BFD_SESSION_STATUS_ADMIN_DOWN:
                    return "ADMIN DOWN";
                case eos::BFD_SESSION_STATUS_NULL:
                    return "NULL";
                default:
                    return "NA";
            }
        }
};

int main(int argc, char ** argv) {
    eos::sdk sdk(AGENT_NAME);
    my_bfd_mon agent(sdk);
    sdk.main_loop(argc,argv);
}