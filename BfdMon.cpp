// Copyright (c) 2018 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.
// Written by: Rob Martin 2018
//
// =====================================
// This is not ready, it is still buggy
// =====================================
//
// author: rmartin
// version: 2.0

#include <eos/agent.h>
#include <eos/sdk.h>
#include <eos/bfd.h>
#include <eos/tracing.h>
#include <ctime>
#include <vector>
#include <sstream>
#include <arpa/inet.h>
#include <syslog.h>

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

        // SDK Function that is called when the agent initializes
        void on_initialized() {
            t.trace0("Initialized");
            status_update("Total BFD Peer/State changes",std::to_string(bfdChanges));
            watch_all_bfd_sessions(true);
            for (eos::agent_option_iter_t opt_obj = agent_mgr->agent_option_iter();opt_obj;opt_obj++) {
                std::string opt_name = opt_obj->c_str();
                std::string opt_value = agent_mgr->agent_option(opt_name);
                on_agent_option(opt_name,opt_value);
            }
            _to_syslog("BFD Agent Initialized");
        }

        // SDK Function that is called when a new agent option is specified
        void on_agent_option(std::string const & optionName, std::string const & value) {
            std::vector<std::string> o_value;
            std::string vrf1;
            std::string oIP;
            std::string oIntf;
            peers tmpPeer;
            o_value = split(value);
            int value_length = o_value.size();
            //oIntf = o_value[1];
            int resIP = _validate_IP(o_value[0]);
            if (resIP == 1) {
                oIP = o_value[0];
            }
            else {
                status_update("Incorrect IP Value for "+optionName,o_value[0]);
                oIP = "";
            }
            if (_checkString(_strLower(o_value[1]),"ethernet")) {
                oIntf = _strCapital(o_value[1]);
            }
            else if (_checkString(_strLower(o_value[1]),"eth")) {
                oIntf = _strCapital(_replace_string(_strLower(o_value[1]),"eth","ethernet"));
            }
            else {
                status_update("Incorrect Interface for " + optionName,o_value[1]);
                oIntf = "";
            }
            if (oIP != "" and oIntf != "") {
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
                peer_list[peer_list_count++] = tmpPeer;
                auto bfd_key = eos::bfd_session_key_t(ip1,vrf1,eos::BFD_SESSION_TYPE_NORMAL,intf1);
                bfd_session_mgr_->session_set(bfd_key);
                for (int i = peer_error_count - 1; i >= 0;i--) {
                    if (peer_error[i][0] == optionName) {
                        status_delete(peer_error[i]);
                        peer_error.erase(peer_error.begin()+i);
                        peer_error_count--;
                        
                    }
                }
            }
            else {
                if (oIP == "") {
                    std::vector<std::string> tmp;
                    tmp.push_back(optionName);
                    tmp.push_back("ip");
                    peer_error.push_back(tmp);
                    peer_error_count++;
                }
                if (oIntf == "") {
                    std::vector<std::string> tmp;
                    tmp.push_back(optionName);
                    tmp.push_back("intf");
                    peer_error.push_back(tmp);
                    peer_error_count++;
                }
            }
            _update_status();
        }

        // SDK Function that is called when a registered BFD session changes status
        void on_bfd_session_status(eos::bfd_session_key_t const& bfdKey, eos::bfd_session_status_t operState){
            bfdChanges++;
            std::string bfdState = _get_status(operState);
            time_t now = time(0);
            std::string l_time_change = ctime(&now);
            std::string bfIP = bfdKey.ip_addr().to_string();
            std::string bfINTF = bfdKey.intf().to_string();
            std::string t_msg = "The State of " +  bfIP + " is now " + bfdState;
            t.trace5(t_msg.c_str());
            status_update("Total BFD Peer/State changes",std::to_string(bfdChanges));
            _to_syslog("Peer " + bfIP + " on " +  bfINTF + " is " + bfdState);
            for (int i = 0; i < peer_list_count; i++) {
                if (peer_list[i].ip == bfIP and peer_list[i].intf == bfINTF) {
                    peer_list[i].stat_chg++;
                    peer_list[i].last = l_time_change;
                }
            }
            _update_status();
        }
    private:
        int bfdChanges = 0; //# of BFD Peer/State changes
        std::vector<std::vector<std::string>> peer_error; //vector array of vector array to keep track of bad option values
        int peer_error_count = 0;
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

        // Function that will write string message to syslog
        void _to_syslog(std::string sys_msg) {
            sys_msg = "%%myBFD-6-LOG: BFD State Change: " + sys_msg;
            syslog(LOG_WARNING,sys_msg.c_str());;
        }

        // Function to verify string is a valid IP address
        int _validate_IP(std::string uIP) {
            struct sockaddr_in sa;
            char str[INET_ADDRSTRLEN];
            int result = inet_pton(AF_INET,uIP.c_str(),&(sa.sin_addr));
            return result;
        }

        // Function to capitalize the first character of a string
        std::string _strCapital(std::string u_input) {
            std::string new_string;
            new_string += toupper(u_input[0]);
            for (int i =1; i < u_input.length();++i) {
                new_string += tolower(u_input[i]);
            }
            return new_string;
        }

        // Function to change string to lowercase
        std::string _strLower(std::string u_input) {
            std::string new_string;
            for (int i = 0; i < u_input.length(); ++i) {
                new_string += tolower(u_input[i]);
            }
            return new_string;
        }

        // Function to check for pattern in string
        bool _checkString(std::string u_input, std::string patt) {
            int pos;
            pos = u_input.find(patt);
            if (pos >= 0) {
                return true;
            }
            else {
                return false;
            }
        }

        // Function to replace substring within a string
        std::string _replace_string(std::string oldString,std::string patt,std::string nValue){
            int posi = 0;
            std::string newString;
            posi = oldString.find(patt);
            newString = oldString.substr(0,posi) + nValue;
            newString += oldString.substr(posi+patt.size(),std::string::npos);
            return newString;
        }

        // Function to split a csv string and return an array
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

        // Function to delete a status value
        void status_delete(std::vector<std::string> s_value) {
            if (s_value[1] == "ip") {
                agent_mgr->status_del("Incorrect IP Value for " + s_value[0]);
            }
            else if (s_value[1] == "intf") {
                agent_mgr->status_del("Incorrect Interface for " + s_value[0]);
            }
            
        }

        // Function to write string/value pair to agent status
        void status_update(std::string s_name,std::string s_value) {
            agent_mgr->status_set(s_name,s_value);
        }

        // Function to update all BFD session status' on the show daemon BfdMon section
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

        // Function to return friendly status for BFD session
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
    openlog("myBFDLog",0,LOG_LOCAL4);
    eos::sdk sdk(AGENT_NAME);
    my_bfd_mon agent(sdk);
    sdk.main_loop(argc,argv);
}