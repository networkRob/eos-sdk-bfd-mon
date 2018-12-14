# EOS SDK BFD Monitor - Python
#### version 1.8
Removed unnecesary mount profile options to SysDB.  The additional profiles that were none existent in prior EOS releases were causing crashes of the agent.
#### version 1.7
Added the ability to leverage an SVI for the neighbor peer connection. The option comand would be inputed as follows:
```
option RTR-02 value 10.0.0.2,Vlan12,default
```
#### version 1.6
Introduced a .swix install for the Python version of the agent.

#### version 1.5
Corrected an issue when the agent initialized with pre-configured option/values in the config file, it wouldn't load those configs.  Also added logic to notify on the `show daemon BfdMon` command that an incorrect IP address or Intf was entered.

#### verson 1.4
This will create an EOS agent to monitor BFD status changes.  As of this moment, when a BFD session status changes, it will create a switch SYSLOG message.  This can be modified further to create a SNMP Trap message to a remote syslog server.
