### EOS SDK BFD Monitor
#### version 1.5
Corrected an issue when the agent initialized with pre-configured option/values in the config file, it wouldn't load those configs.  Also added logic to notify on the `show daemon BfdMon` command that an incorrect IP address or Intf was entered.

#### verson 1.4
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
option {peerX} value {ip_of_bfd_peer},{local_interface_connecting_to_peer},{vrf}
```
**`peerX` needs to be a unique identifier for each peer*

***To see what unique peer identifiers have been created, enter `show daemon BfdMon`*

Example
```
config
daemon BfdMon
option RTR-02 value 10.0.0.2,Ethernet1,default
```
6. To make this agent persist a reboot/power-cycle.  We will need to create a `rc.eos` file in the `/mnt/flash` directory of the switch.  The contents for `rc.eos` are as follows:
```
#!/bin/bash
cp /mnt/flash/BfdMon.mp /usr/lib/SysdbMountProfiles/BfdMon
```


#### Sample output of `show daemon BfdMon`
```
7280-rtr-01(config-daemon-BfdMon)#show daemon
Agent: BfdMon (running with PID 4877)
Configuration:
Option       Value
------------ --------------------------
RTR-02       10.0.0.2,Ethernet1,default

Status:
Data                                                        Value
----------------------------------------------------------- ------------------------
Total BFD Peer/State changes                                1
[RTR-02] Last Status Change: [10.0.0.2 on Ethernet1]        Fri Aug 10 15:23:56 2018
[RTR-02] Status Change Total: [10.0.0.2 on Ethernet1]       1
[RTR-02] Status: [10.0.0.2 on Ethernet1]                    UP
```