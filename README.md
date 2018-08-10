### EOS SDK BFD Monitor
This will create an EOS agent to monitor BFD status changes.

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
