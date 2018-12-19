# EOS SDK BFD Monitor
There are 2 versions of this sdk agent.  One written in Python, and the other in c++.  Below are their current status:
1. Python: source/Python/BfdMon [stable]
- Supports Ethernet and Vlan interfaces for BGP neighbors for BFD monitoring.
2. C++: source/C++/BfdMon.cpp [stable]
- Supports Ethernet and Vlan interfaces for BGP neighbors for BFD monitoring.

The .swix extensions can be found in the [release/](release/) directory.

## Switch Setup 

### Install
1. Copy `BfdMon-x.x.x-x.swix` to `/mnt/flash/` on the switch or to the `flash:` directory.
2. Copy and install he `.swix` file to the extensions directory from within EOS.  Below command output shows the copy and install process for the extension.
```
7280-rtr-01#copy flash:BfdMon-1.6.0-1.swix extensions:
Copy completed successfully.
7280-rtr-01#show extensions
Name                         Version/Release      Status      Extension
---------------------------- -------------------- ----------- ---------
BfdMon-1.6.0-1.swix          1.6.0/1              A, NI       1
TerminAttr-1.5.0-1.swix      v1.5.0/1             A, I        24


A: available | NA: not available | I: installed | NI: not installed | F: forced
7280-rtr-01#extension BfdMon-1.6.0-1.swix
7280-rtr-01#show extensions
Name                         Version/Release      Status      Extension
---------------------------- -------------------- ----------- ---------
BfdMon-1.6.0-1.swix          1.6.0/1              A, I        1
TerminAttr-1.5.0-1.swix      v1.5.0/1             A, I        24


A: available | NA: not available | I: installed | NI: not installed | F: forced
```
3. In order for the extension to be installed on-boot, enter the following command:
```
7280-rtr-01#copy extensions: boot-extensions
```

## BFD Agent Configuration
1. Verify BFD is running on the switch and remote switch.  You can verify with the following EOS command: `show bfd neighbors`
2. In EOS config mode perform the following steps: 
```
config
daemon BfdMon
exec /usr/bin/BfdMon
no shut
```
3. In order for this agent to receive updates on BFD sessions, the following commands will need to be taken:
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