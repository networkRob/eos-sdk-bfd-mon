### EOS SDK BFD Monitor
#### verson 1.2
This will create an EOS agent to monitor BFD status changes.  As of this moment, when a BFD session status changes, it will create a switch SYSLOG message.  This can be modified further to create a SNMP Trap message to a remote syslog server.

#### Caveats
Right now this agent assumes the BFD session belongs to the `default` vrf.  The ability to specify th vrf will be added in the future.

#### Setup
1. Copy `BfdMon` and `profileBFD` to `/mnt/flash/` on the switch
2. Run the following command while on bash for the switch: `/mnt/flash/./profileBFD`
3. Verify BFD is running on the switch and remote switch.  You can verify with the following EOS command: `show bfd neighbors`
4. In EOS config mode perform the following steps:
```config
daemon BfdMon
exec /mnt/flash/BfdMon
no shut
```
5. In order for this agent to receive updates on BFD sessions, the following commands will need to be taken:
```
config
daemon BfdMon
option name value peerX-ip {ip_of_bfd_peer}
option name value peerX-intf {local_interface_connecting_to_peer}
```
**The `X` in `peerX` needs to be a unique identifier for each peer*

***To see what unique peer identifiers have been created, enter `show daemon BfdMon`*

#### Sample output of `show daemon BfdMon`
```7280-rtr-01(config-daemon-BfdMon)#show daemon
Agent: BfdMon (running with PID 20886)
Configuration:
Option       Value
------------ ---------------
name         peer1-intf eth1

Status:
Data                                                       Value
---------------------------------------------------------- ------------------------
Total BFD Peer/State changes                               1
[peer1] Last Status Change: [10.0.0.2 on Ethernet1]        Fri Aug 10 13:50:34 2018
[peer1] Status Change Total: [10.0.0.2 on Ethernet1]       1
[peer1] Status: [10.0.0.2 on Ethernet1]                    UP
```