var test_server = require("bls");

var config = {
	log_conf_path : "test/comlog-1.conf",
	max_client_num : 20,
	port : 7001,
	ping_pong_time : 10,
};

test_server.start_server(config, function(client){
	console.log("client come on! id: %s", client.client_id);

	client.on("connect", function(trans_id, connect_info)
	{
		console.log("new client connect. tsid: %d connect_info: %j", 
			trans_id, connect_info);
		client.accept(true);
		//client.accept(false, "NetConnection.Connect.Rejected", "hehe");
	});

	client.on("close", function(close_status)
	{
		console.log("client close ", close_status);
	});

	client.on("publish", function(trans_id, cmd_objs, stream_name)
	{
		console.log("client call publish. tsid: %d cmd_objs: %j stream_name: %s",
			trans_id, cmd_objs, stream_name);

		client.publish(trans_id, stream_name);
	});

	client.on("play", function(trans_id, cmd_obj, stream_name){
		console.log("client call play. tsid: %d cmd_objs: %j stream_name: %s",
			trans_id, cmd_obj, stream_name);

		client.play(trans_id, stream_name);
	});

	client.on("unplay", function(){
		console.log("client unplay stream......");
	});

	client.on("unpublish", function(){
		console.log("client unpublish stream......");
	});

	// setTimeout(function(){
	// 	client.close();
	// }, 5000);
});

setTimeout(function(){

	test_server.remote_connect("127.0.0.1", 8955, function(edge_connect){
		if(edge_connect)
		{
			console.log("connect remote server success.")
			edge_connect.connect({info:"hehe"}, function(results){
				console.log("send connect to remote server. recv:");
				console.dir(arguments);

				edge_connect.push("78c1f9ba124611e4815aac853dd1c904");
			});
		}
		else
		{
			console.log("connect remote server fail");
		}
	});

}, 1000);