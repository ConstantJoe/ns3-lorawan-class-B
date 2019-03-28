# ns3-lorawan-class-B

Modifys imec iDLab's ns-3 implementation of LoRaWAN to include Class B mode.

This is temporarily *not* a fork because I already have one fork of the imec ns-3 implementation.

***

The original repository can be found here:

https://github.com/imec-idlab/ns-3-dev-git/tree/lorawan

Just the modified LoRaWAN ns-3 library can be found in the master branch. The library as part of a full ns-3 install (which can be downloaded and ran straight away) can be found in the lorawan branch.

***

# What is here:

An extention of imec iDLab's ns-3 implementation of LoRaWAN to include Class B mode, including:

For the Network Server

- generation and sending of beacon frames

- calculation of ping slots for each Class B device

- the transmission of downlink frames in those slots when data is pending and the duty cycle limits allow. 

- the detection of the Class B bit in the uplink frame header to signify to the NS whether to calculate ping slots and send downlink frames to a device

For the End Devices: 

- the waking up and reception of beacon frames when in Class B mode

- the use of the timestamp in the beacon frame to calculate a device's own ping slots (in the same way as the NS)

- the waking up in those ping slots to potentially receive additional downlink frames 

***

# What is not here (i.e. TO DO):

LoRaWAN in general (TODOs inherited from imec library):

- The LoRaWAN Join Procedure

- Encryption / Decryption

- Class C

- many other things - see imec original repository for details

Class B related:

- Class B transition (i.e. all nodes start in Class A mode, have to detect beacon frames in order to transition). Currently devices just either start in Class A or Class B mode.

- The widening of RX slots in Class B beaconless mode.

- A satisfactory way of modeling time drift in the internal clocks of end devices in ns-3 (right now, if a certain number of beacon frames are missed, the device is just presumed to have gone out of sync)

- multicast support

Also have TO DO:

- more Class B-specific examples

- more Class B-specific tests

- an update of the documentation

- move this whole repository to be a fork of the imec one.
