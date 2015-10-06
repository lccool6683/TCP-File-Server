Clemens Lo	A00863045
COMP 7005 Assignment #1 - TCP File Server & Client


This assignemnt meets all requirments.



- Server can only handle single client at the same time then close connection and keep waiting for new connection.
- Default port is 7005
- Both Server and client will overwrite file if there is already exist.

How to run the appliction:
----------------------------------------------------------------------
./tserver.exe [port]
./tclient.exe [GET/SEND] 127.0.0.1 [port]