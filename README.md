# bls
A rtmp server framework for Nodejs. This server is developed in libuv I/O framework which is used by Nodejs. So it's performace in case of a large number of clients push or pull stream data currently is very good. At the same time, you can add custom logics, such as auth/cluster, in this framework easily with js api. A variety of API are provided to get infomations about RTMP stream, include the quality of one stream.

> Note: Not the full RTMP protocal is supported. But the basic function of live play has been realised.

##Requirement
- only support nodejs >=0.10 <0.12
- Linux
