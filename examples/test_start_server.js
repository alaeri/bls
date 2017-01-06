var test_server = require("bls");
var fs = require("fs");

var config = {
    log_conf_path : "test/comlog.conf",
    max_client_num : 2000,
    port : 8956,
    ping_pong_time : 10,
};

var play_dict = {};
var publish_dict = {};

test_server.start_server(config, function(client){
    console.log("client come on! id: %s", client.client_id);

    client.on("connect", function(trans_id, connect_info)
    {
        console.log("new client connect. %s tsid: %d connect_info: %j", 
            client.client_id, trans_id, connect_info);
        client.accept(true);
        //client.accept(false, "NetConnection.Connect.Rejected", "hehe");
    });

    client.on("close", function(close_status)
    {
        console.log("%s client close ", client.client_id, close_status);
    });

    client.on("publish", function(trans_id, cmd_objs, stream_name)
    {
        console.log("client call publish. tsid: %d cmd_objs: %j stream_name: %s",
            trans_id, cmd_objs, stream_name);

        client.publish(trans_id, stream_name);

        client.publish_stream_name = stream_name;

        publish_dict[stream_name] = client;
    });

    client.on("play", function(trans_id, cmd_obj, stream_name){
        console.log("client call play. tsid: %d cmd_objs: %j stream_name: %s",
            trans_id, cmd_obj, stream_name);

        client.play(trans_id, stream_name);
        
    });

    client.on("onMetaData", function(meta_data){
        console.log("get metadata %j", meta_data);
    })

    client.on("unplay", function(){
        console.log("client unplay stream......");
    });

    client.on("unpublish", function(){
        console.log("client unpublish stream......");
    });

    client.on("ping_pong_request", function(delay, recv_sum){
        console.log("get pong response! %d %d", delay, recv_sum);
    });

    client.on("play_stop_event", function(stream_name)
    {
        console.log("get play stop event %j", stream_name);
        client.close();
    });

    var video_flag = 0;

    client.enable_av_cb(function(av_type, is_keyframe, is_avc_sh, timestamp, size){
        console.dir(arguments);
        console.log(client.av_buffer);

        fs.writeFileSync("./vide_data_"+video_flag, client.av_buffer.slice(0, size));
        video_flag += 1;
    });

    client.on("av", function(av_type, is_keyframe, is_sh, timestamp, size){
        console.dir(arguments);
        // console.log(client.av_buffer);

        // fs.writeFileSync("./vide_data_"+video_flag, client.av_buffer.slice(0, size));
        // video_flag += 1;
        var player_ws = play_dict[client.publish_stream_name];
        if(player_ws)
        {
            if(is_keyframe)
            {
                player_ws.had_key_frame = true;
            }
            if(player_ws.had_key_frame)
            {
                player_ws.emit(av_type, timestamp, is_sh, client.av_buffer.slice(0, size));
            }
        }
    });

    // setTimeout(function(){
    //     console.log("get aac sh %d", client.get_aac_sh());
    //     console.log(client.av_buffer);

    //     console.log("get avc sh %d", client.get_avc_sh());
    //     console.log(client.av_buffer);
    // }, 2000);
});
