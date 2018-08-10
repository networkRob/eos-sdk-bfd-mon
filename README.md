### EOS SDK BFD Monitor
#### verson 1.3
This will create an EOS agent to monitor BFD status changes.  As of this moment, when a BFD session status changes, it will create a switch SYSLOG message.  This can be modified further to create a SNMP Trap message to a remote syslog server.

#### Setup
1. Copy `BfdMon`, `BfdMon.mp` and `profileBFD` to `/mnt/flash/` on the switch
2. Run the following command on the switch: 
```
bash /mnt/flash/profileBFD
```
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
option name value peerX {ip_of_bfd_peer},{local_interface_connecting_to_peer},{vrf}
```
**`peerX` needs to be a unique identifier for each peer*

***To see what unique peer identifiers have been created, enter `show daemon BfdMon`*

Example
```
config
daemon BfdMon
option name value Peer1 10.0.0.2,Ethernet1,default
```
6. To make this agent persist a reboot/power-cycle.  We will need to create a `rc.eos` file in the `/mnt/flash` directory of the switch.  The contents for `rc.eos` are as follows:
```
#!/bin/bash
cp /mnt/flash/BfdMon.mp /usr/lib/SysdbMountProfiles/BfdMon
```


#### Sample output of `show daemon BfdMon`
```
7280-rtr-01(config-daemon-BfdMon)#show daemon
Agent: BfdMon (running with PID 27318)
Configuration:
Option       Value
------------ ---------------------------
name         Peer1 10.0.0.2,eth1,default

Status:
Data                                                       Value
---------------------------------------------------------- ------------------------
Total BFD Peer/State changes                               1
[Peer1] Last Status Change: [10.0.0.2 on Ethernet1]        Fri Aug 10 14:26:42 2018
[Peer1] Status Change Total: [10.0.0.2 on Ethernet1]       1
[Peer1] Status: [10.0.0.2 on Ethernet1]                    UP
```