# bls
BLS is a rtmp server framework for Nodejs. This server is developed in libuv I/O framework which is used by Nodejs. So it's performace in case of a large number of clients push or pull stream data concurrently is very good. At the same time, you can add custom logics, such as auth/cluster, in this framework easily with js api. A variety of API are provided to manage RTMP stream, such as open and close a stream, get the quality of one stream.

> Note: Not the full RTMP protocal is supported. But the basic function of live play has been realised.

##Requirement
- only support nodejs >=0.10 <0.12 just now
- Linux 64 bit

##Example
```javascript
//simple_server.js

var server = require("bls");

var config = {
    //log file path bls write its own log to
    log_path : "log/bls.log",

    //trace:0 debug:1 info:2 warn:3 err:4 critical:5 off:6
    //if you use low level, bls will be more busy
    log_level : 1,

    //the maximum number of clients connect to server concurrently
    max_client_num : 2000,

    //port listen to
    port : 8956,

    //the interval seconds bls uses to send heartbeat msg to clients
    ping_pong_time : 10,
};

//start listen and serve as RTMP server
//cb func is called when a client connects(tcp connect) to server
server.start_server(config, function(client){
    console.log("client come on! id: %s", client.client_id);

    //the callback func when this client sends RTMP connect command
    client.on("connect", function(trans_id, connect_info)
    {
        console.log("new client connect. %s tsid: %d connect_info: %j", 
            client.client_id, trans_id, connect_info);

        client.accept(true);
        //client.accept(false, "NetConnection.Connect.Rejected", "reject");
        
        //you can send any command to client
        //if you need result from client, the callback function must be set
        client.call("needResult", [{}, {data:66}], function(res_cmd, res_args){
            console.log("get result from client %s %s", res_cmd, res_args);
        });
    });

    //this client leave
    client.on("close", function(close_status)
    {
        console.log("%s client close ", client.client_id, close_status);
    });

    //register a cb func when this client wants to publish a stream.
    //note: bls just allows one client publishs one stream now.
    client.on("publish", function(trans_id, cmd_objs, stream_name)
    {
        console.log("client call publish. tsid: %d cmd_objs: %j stream_name: %s",
            trans_id, cmd_objs, stream_name);

        //if you allow this client to publish stream with stream_name, jus need to call publish function
        //trans_id must be same with trans_id in cb arguments
        //you can custom the stream name which bls uses to publish
        client.publish(trans_id, stream_name);
    });

    //register a cb func when this client wants to play a stream
    //note: bls just allows one client play one stream now.
    client.on("play", function(trans_id, cmd_obj, stream_name){
        console.log("client call play. tsid: %d cmd_objs: %j stream_name: %s",
            trans_id, cmd_obj, stream_name);

        //trans_id must be same with the cb arguments
        //you can choose a stream name for this client, not must be same with the client wants
        client.play(trans_id, stream_name);
        
    });

    //when client publishs a stream, there will be a meta data in stream data
    //meta data size should not be bigger than MAX_BUFFER_LEN, default is 2KB
    client.on("onMetaData", function(meta_data){
        console.log("get metadata %j", meta_data);
    })

    //when this client call stop play command
    client.on("unplay", function(){
        console.log("client unplay stream......");
    });

    //when this client call unplish, which means this client wants stop publish stream
    client.on("unpublish", function(){
        console.log("client unpublish stream......");
    });

    //bls sends heartbeat to client with seconds interval, which is indicated in config
    //when client send back pong msg, which is required, this cb func will be called
    //delay indicates the transport delay between bls and client
    client.on("ping_pong_request", function(delay, recv_sum){
        console.log("get pong response! %d %d", delay, recv_sum);
    });

    //listen to custom command from client, so client can send custom data to bls
    client.on("customCmd", function(trans_id, cmd_obj, data){
        console.log("get user custom command. %s %s %s", trans_id, cmd_obj, data);

        var result = "result data";

        //you can answer client with "_result" or "_error"
        //trans_id must be same with the one in cb func arguments
        client.result("_result", trans_id, result);
    });
});
```

##API

###Function: start_server(config, cb(client))
start a rtmp server
- **config**  `[object]` configuration for server, including log path / port and so on. 
- **cb** `[function]` callback function which is triggled when a new client come on TCP level. In this function, you can register many event callbacks for client, and control this client
	- **client** `[BlsClient]` callback argument. stand for a client, see detail about [BlsClient]()

configure items:

| Item      |    required | type | description |
|-----------|-------------|------|-------------|
| log_path |  yes | string | log file path for bls to record detail info  |
| log_level | yes | number | trace:0 debug:1 info:2 warn:3 err:4 critical:5 off:6 |
| max_client_num | yes | number | how many clients the server can hold at the same time |
| port| yes | number | the port server listens to |
| ping_pong_time | no | number | the interval seconds server sends ping package for detecting whether this client is alive or not. Default 10. |

###Function:remote_connect(ip, port, cb(client))
Create a TCP connection to another bls server.
- **ip** `[string]` remote bls server ip
- **port** `[number]` remote bls server port
- **cb** `[function]` callback function
	- **client** `[BlsClient]` callback argument. If connect fail client will be None, else it will be a BlsClient instance. You should call client.connect() method to complete RTMP connect.

Example
```javascript
var bls = require("bls");
bls.remote_connect("127.0.0.1", 8955, function(edge_connect){
    if(edge_connect)
    {
        console.log("connect remote server success.")
        edge_connect.connect({info:"auth"}, function(flag, args){
            console.log("send connect to remote server. flag:%s args:%j", flag, args);
        });
    }
    else
    {
        console.log("connect remote server fail");
    }
});
```
###Var:MAX_BUFFER_LEN
Indicate the max size of command data.

###Class:BlsClient
BlsClient instance stands for a client that connects to server. A lot of events can be catched from a client, and you can control this client through APIs. BlsClient inherits from Emitter.

####BlsClient.prototype.accept(allow, code, descript)
Decide whether accept this client in RTMP protocol.
- **allow** `[boolean]` true means accept, false means reject
- **code** `[string]` RTMP connect reject code. If allow is true, code is  NetConnection.Connect.Success default. Otherwise it is [NetConnection.Connect.Error|NetConnection.Connect.Fail|...]
- **descript** `[string]` description about rejection result.

```javascript
    client.on("connect", function(trans_id, connect_info)
    {
        console.log("new client connect. %s tsid: %d connect_info: %j",
            client.client_id, trans_id, connect_info);

		//accept
        client.accept(true);
        ////reject
        //client.accept(false, "NetConnection.Connect.Rejected", "reject");
        });
    });
```

####BlsClient.prototype.call(cmd_name, args_array, cb_func(result_flag, args))
Send user custom command to client. If result is not needed, cb_func should be None.
- **cmd_name** `[cmd_name]` user custom command name
- **args_array** `[array]` command args. 
- **cb_func** `[function]` callback function when client sends result according to this command.
	- **result_flag** `[string]` "_result" or "_error" recv from client.
	- **args** `[array]` result data recv from client
	
####BlsClient.prototype.close()
Close this client connection.

####BlsClient.prototype.edge(stream_name, cb())
This method is made for cluster. Local BLS can pull stream data from a remote BLS server as a source. Then player clients can play this stream from local BLS. The client must be producted from `remote_connect` function.
- **stream_name** `[string]` the name of stream you want to pull from remote BLS. And this stream_name will be local stream name.
- **cb** `[function]` called when pull finish, which means players can play the stream name from local BLS from now on.
```javascript
bls.remote_connect("127.0.0.1", 8956, function(edge_connect){
    if(edge_connect)
    {
        console.log("connect remote server success.")
        edge_connect.connect({info:"hehe"}, function(results){
            console.log("send connect to remote server. recv:");
            console.dir(arguments);

            edge_connect.edge("78c1f9ba124611e4815aac853dd1c904", function(){
                console.log("edge complete");
            });
        });
    }
    else
    {
        console.log("connect remote server fail");
    }
});
```

####BlsClient.prototype.get_aac_sh()
return aac sequence header data recieved from client.

####BlsClient.prototype.get_avc_sh()
return avc sequence header data recieved from client.

####BlsClient.prototype.is_closed()
return True if client is not alive.

####BlsClient.prototype.play(trans_id, stream_name)
Allow client to play one stream.
>**Note**: One client can only play one stream now.
>**Note**: If this stream name is not publishing, the player will never get stream data even if this stream publish later.

- **trans_id** `[number]` must be same with trans id in play event
- **stream_name** `[number]` indicates which stream is passed to client. The stream_name must be same with the publish one. But it is not necessary same with stream name in play event.

####BlsClient.prototype.publish(trans_id, stream_name)
Allow client to publish one stream

>**Note**: One client can only publish one stream at the same time.

- **trans_id** `[number]` must be same with trans id in publish event.
- **stream_name** `[number]` indicates the stream name to publish with this client.

####BlsClient.prototype.push(stream_name, cb)
This method is made for cluster. Local BLS can push stream data to a remote BLS server as a source. Then player clients can play this stream from remote BLS. The BLS client must be producted from `remote_connect` function.

- **stream_name** `[string]` the name of stream you want to push to remote BLS. 
- **cb** `[function]` called when push finish, which means players can play the stream name from remote BLS from now on.

####BlsClient.prototype.result(result_flag, transid, args)
Send result to client according to the command received from client.

- **result_flag** `[string]` "_result" or "_error"
- **transid** `[number]` must be same with transid in command event
- **args** `[array]` result data

Example:
```javascript
    //listen to custom command from client, so client can send custom data to bls
    client.on("customCmd", function(trans_id, cmd_obj, data){
        console.log("get user custom command. %s %s %s", trans_id, cmd_obj, data);

        var result = ["result data"];

        //you can answer client with "_result" or "_error"
        //trans_id must be same with the one in cb func arguments
        client.result("_result", trans_id, result);
    });
```

####BlsClient.prototype.unpublish()
Client stop publishing stream.

####Event:connect(trans_id, connect_info)
Emitted when a client send RTMP connect command to BLS.
- **trans_id** `[number]` rtmp protocol needs.
- **connect_info** `[object]` connect information recieved from client.

####Event:close(close_status)
Emitted when client leave.

####Event:publish(trans_id, cmd_objs, stream_name)
Emitted when client wants to publish a stream

####Event:play(trans_id, cmd_obj, stream_name)
Emitted when client wants to play a stream

####Event:onMetaData(meta_data)
Emitted when recieved meta data from client in process of publish stream.

####Event:unplay()
Emitted when client stop play stream.

####Event:unpublish()
Emitted when client stop publish stream.

####Event:ping_pong_request(delay, recv_sum)
Emitted when client send pong response to BLS.
- **delay** `[number]` delay after BLS send ping request to client.
- **recv_sum** `[number]` how many bytes recved from client totally.

####Event:{customCommand}(trans_id, cmd_obj, data)
Emitted when receive user custom command.
