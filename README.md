# UVR_CAN

C and C++ routines to talk to TA UVR 1611 controllers via the CAN bus. The little helpers require to have libcanopen installed.

For those who would like to access other objects keep in mind to do a "verbindungsaufbau/connection setup" to get a valid cob_id.

scancanbus listens for NMT messages on the wire to understand which devices are there. It accesses the vendor/product objects to detect a UVR 1611.

readViaCAN retrieves the actual values of the controller and can access its sensor/output names.

Weaknesses:
Some functions send a series of SDO requests which seems to be to much for older UVR1611. They tend to block from time to time. I dont have an explanation for this. In practice it looks fairly robust to just pack ~8 SDOs in tighter series

You find related info on staircaseblog.blogspot.com
