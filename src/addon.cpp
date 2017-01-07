/*
 * addon.cpp
 *
 *  Created on: 2014-10-31
 *      Author: chenyuliang01
 *
 * Copyright (c) 2014, chenyuliang01@baidu.com baiducam All Rights Reserved.
 */

#include <string>
#include <node/node.h>
#include <node/node_buffer.h>
#include <RtmpServer.h>
#include <RtmpClient.h>
#include <RtmpChunkPool.h>
#include <BlsMessage.h>
#include <RtmpProtocol.h>
#include <BlsLogger.h>
#include <utilities.h>
#include <BlsSource.h>

using namespace v8;

Persistent<Function> g_connect_cb;

void bls_on_accept(void *c)
{
    ((RtmpClient *) c)->start();
}

/**
 * 用配置文件路径初始化日志模块
 * @param args[0] 配置文件路径
 * @return
 */
Handle<Value> init_logger(const Arguments& args)
{
    HandleScope scope;
    String::Utf8Value temp_str(args[0]->ToString());
    int level = int(args[1]->NumberValue());

    init_bls_logger((char *) *temp_str, level);

    return scope.Close(Undefined());
}

Handle<Value> write_log(const Arguments& args)
{
    HandleScope scope;
    String::Utf8Value temp_str(args[0]->ToString());
    char *log_content = (char *) *temp_str;
    
    while(*log_content != 0)
    {
        if(*log_content == '%')
        {
            *log_content = '@';
        }
        ++log_content;
    }

    BMS_LOG((char *) *temp_str);

    return scope.Close(Undefined());
}

/**
 * 开启服务器，监听端口，根据conf初始化运行参数
 * @param args[0] object 配置参数
 * @return
 */
Handle<Value> start_server(const Arguments& args)
{
    HandleScope scope;

    rtmp_config_t conf;
    RtmpServer *server;

    Local<Object> config_obj = args[0]->ToObject();

    /*
     * 读取配置信息
     */
    String::Utf8Value temp_str(
            config_obj->Get(String::New("log_conf_path"))->ToString());
    conf.log_conf_path = (char *) *temp_str;

    conf.log_level = config_obj->Get(
        String::New("log_level"))->ToInt32()->Int32Value();

    if (config_obj->Has(String::New("max_client_num")))
    {
        conf.max_client_num = config_obj->Get(
                String::New("max_client_num"))->ToInt32()->Int32Value();
    }
    if (config_obj->Has(String::New("port")))
    {
        conf.port = config_obj->Get(
                String::New("port"))->ToInt32()->Int32Value();
    }
    if (config_obj->Has(String::New("ping_pong_time")))
    {
        conf.ping_pong_time = config_obj->Get(
                String::New("ping_pong_time"))->ToInt32()->Int32Value();
    }

    //初始化日志模块
    init_bls_logger(conf.log_conf_path, conf.log_level);

    //初始化chunk池，预先分配内存资源
    init_chunk_pool(conf.chunk_bucket_size, conf.chunk_pool_size);

    //初始化消息处理映射表
    init_type_message_map();

    //初始化source池
    init_source_pool();

    server = new RtmpServer(conf);
    server->register_on_accept(bls_on_accept);
    server->start();

    g_connect_cb = Persistent<Function>::New(Local<Function>::Cast(args[1]));

    return scope.Close(Undefined());
}

/**
 * 绑定read buffer 和 write_buffer 用于将底层的msg封装之后交给js层面
 * 同时在js中初始化msg内容后交给底层发送出去
 * @param args[0] client 底层对应的client实例
 * @param args[1] read_buffer 上层读取msg用的buffer
 * @param args[2] write_buffer 底层从buffer中读取要发送的msg
 * @return
 */
Handle<Value> bind_read_write_buf(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);

    if (!node::Buffer::HasInstance(args[1]))
    {
        v8::ThrowException(
                v8::Exception::TypeError(v8::String::New("bad arguments")));
    }

    client->node_buf_len = node::Buffer::Length(args[1]->ToObject());
    client->node_read_buf = (uint8_t *) (node::Buffer::Data(args[1]->ToObject()));
    client->node_write_buf = (uint8_t *) node::Buffer::Data(args[2]->ToObject());
    client->node_av_buf = (uint8_t *) node::Buffer::Data(args[3]->ToObject());


    return scope.Close(Undefined());
}

/**
 * 底层收到msg后回调上层函数，交给js处理
 * @param args[0] client 底层对应的client实例
 * @param args[1] cb_func 上层处理msg的回调函数
 * @return
 */
Handle<Value> bind_on_msg_cb(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    client->node_on_msg_cb = Persistent<Function>::New(Local<Function>::Cast(args[1]));

    return scope.Close(Undefined());
}

/**
 * 触发底层上抛视频数据的接口
 * @param args[0] client 底层对应的client实例
 * @param args[1] cbfunc 上层处理msg的回调函数
 * @return
 */
Handle<Value> enable_throwup_av(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    client->node_on_av_cb = Persistent<Function>::New(Local<Function>::Cast(args[1]));
    client->enable_video_up = true;

    return scope.Close(Undefined());
}

/**
 * 关闭底层上抛视频数据的接口
 * @param args[0] client 底层对应的client实例
 * @return
 */
Handle<Value> disable_throwup_av(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    client->enable_video_up = false;

    return scope.Close(Undefined());
}

/**
 * 底层client断开链接，通知js处理
 * @param args[0] client 底层对应的client实例
 * @param args[1] cb_func 上层处理close的回调函数
 * @return
 */
Handle<Value> bind_on_close_cb(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    client->node_on_close_cb = Persistent<Function>::New(Local<Function>::Cast(args[1]));

    return scope.Close(Undefined());
}

/**
 * 获取aac sequence header，拷贝到av_buffer中
 * @param args[0] client实例
 * @return
 */
Handle<Value> get_aac_sh_data(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;
    int ret = -1;

    if (protocol->publish_source)
    {
        ret = protocol->publish_source->source->copy_aac_sh_data(client->node_av_buf);
    }

    return scope.Close(Number::New(ret));
}

/**
 * 获取avc sequence header，拷贝到av_buffer中
 * @param args[0] client实例
 * @return
 */
Handle<Value> get_avc_sh_data(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;
    int ret = -1;

    if (protocol->publish_source)
    {
        ret = protocol->publish_source->source->copy_avc_sh_data(client->node_av_buf);
    }

    return scope.Close(Number::New(ret));
}

/**
 * js中调用，发送message给客户端，在js中制定msg的类型/streamid等信息
 * @param args[0] client 底层对应的client实例
 * @param args[1] buffer 封装了message的内容
 * @param args[2] number streamid
 * @param args[3] number 时间戳
 * @param args[4] number 类型
 * @param args[5] number chunkid，默认为3
 * @return
 */
Handle<Value> write_message_buf(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    uint8_t *write_buf = (uint8_t *) (node::Buffer::Data(args[1]->ToObject()));
    size_t buf_len = node::Buffer::Length(args[1]->ToObject());
    uint32_t stream_id = args[2]->Uint32Value();
    uint32_t timestamp = args[3]->Uint32Value();
    uint8_t type_id = args[4]->Uint32Value();
    uint32_t chunk_id = 3;
    chunk_chain_t temp_chain;

    if(NULL == client->uv_client)
    {
        CLIENT_WARNING(client, "client already closed, return!");
        return scope.Close(Undefined());
    }

    if (args.Length() == 6)
    {
        chunk_id = args[5]->Uint32Value();
    }

    CLIENT_NOTICE(client, "nodejs send message to client."
            " streamid: %u timestamp: %u typeid: 0x%2x chunk_id: %u",
            stream_id, timestamp, type_id, chunk_id);

    encode_buf_to_chunk_chain(temp_chain, write_buf, buf_len, stream_id,
            timestamp, type_id, chunk_id);

    write_chunk_chain(client, temp_chain, false);

    return scope.Close(Undefined());
}

/**
 * 控制底层逻辑在protocol里发布一个流
 * @param args[0] client
 * @param args[1] stream_name 底层用于发布的流名字
 * @return
 */
Handle<Value> publish_stream(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;
    v8::String::Utf8Value node_stream_name(args[1]->ToString());
    std::string stream_name = std::string(*node_stream_name);

    if(NULL == client->uv_client)
    {
        CLIENT_WARNING(client, "client already closed, return!");
        return scope.Close(Boolean::New(false));
    }

    CLIENT_TRACE(client, "publish stream! stream_name: %s", stream_name.c_str());

    source_bucket_t *source = get_publish_source(stream_name);
    if (NULL != source)
    {
        protocol->publish_source = source;
        return scope.Close(Boolean::New(true));
    }
    else
    {
        CLIENT_WARNING(client, "publish error. no source for %s. close it! haha",
                stream_name.c_str());

        return scope.Close(Boolean::New(false));
    }
}

/**
 * 控制底层停止发布一个流
 * @param args[0] client
 * @return
 */
Handle<Value> unpublish_stream(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;
    source_bucket_t *source = protocol->publish_source;

    if(NULL == client->uv_client)
    {
        CLIENT_WARNING(client, "client already closed, return!");
        return scope.Close(Undefined());
    }

    if (NULL != source)
    {
        collect_source(source);
        protocol->publish_source = NULL;
    }

    return scope.Close(Undefined());
}

Handle<Value> send_stream_eof(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;

    protocol->send_stream_eof();

    return scope.Close(Undefined());
}

Handle<Value> send_stream_begin(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;

    protocol->send_stream_begin();

    return scope.Close(Undefined());
}

/**
 * 控制底层播放一个流
 * @param args[0] client
 * @param args[1] 用于播放的streamid
 * @param args[2] stream_name
 * @return
 */
Handle<Value> play_stream(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;
    uint32_t stream_id = args[1]->Uint32Value();
    v8::String::Utf8Value node_stream_name(args[2]->ToString());
    std::string stream_name = std::string(*node_stream_name);
    bls_consumer_t *consumer = NULL;

    if(NULL == client->uv_client)
    {
        CLIENT_WARNING(client, "client already closed, return!");
        return scope.Close(Undefined());
    }

    CLIENT_TRACE(client, "play stream! stream_name: %s", stream_name.c_str());

    protocol->send_stream_begin();

    source_bucket_t *source = get_play_source(stream_name);
    if (NULL != source)
    {
        consumer = init_consumer(client, stream_name, stream_id,
                source->source->video_chunkid, source->source->audio_chunkid);
        source->source->add_consumer(consumer);

        protocol->play_consumer = consumer;
    }

    return scope.Close(Undefined());
}

/**
 * 控制底层停止播放一个流
 * @param args[0] client
 * @return
 */
Handle<Value> unplay_stream(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);
    RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;
    bls_consumer_t *consumer = protocol->play_consumer;

    if(NULL == client->uv_client)
    {
        CLIENT_WARNING(client, "client already closed, return!");
        return scope.Close(Undefined());
    }

    if (NULL != consumer)
    {
        free_consumer(consumer);
        protocol->play_consumer = NULL;
    }

    return scope.Close(Undefined());
}

/**
 * 控制底层关闭一个客户端（异步）
 * @param args[0] client
 * @return
 */
Handle<Value> close_client(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);

    //RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;
    //bls_delete(protocol);

    if(NULL == client->uv_client)
    {
        CLIENT_WARNING(client, "client already closed, return!");
        return scope.Close(Undefined());
    }

    CLIENT_TRACE(client, "close client in nodejs");

    client->close();

    return scope.Close(Undefined());
}

/**
 * 建立远端链接的回调函数
 * @param r_rtmp_client 失败为NULL，成功为一个RtmpClient的指针
 * @param data nodejs里的回调函数
 */
void _node_remote_connect_cb(RtmpClient *r_rtmp_client, v8::Persistent<
        v8::Function> data)
{
    if (NULL != r_rtmp_client)
    {
        //握手过程通过私有协议已经结束，可以直接开始协议交互
        RtmpProtocol *protocol = new RtmpProtocol(r_rtmp_client);
        r_rtmp_client->protocol = (void *) protocol;

        protocol->start();

        //回调nodejs函数
        v8::Local<v8::Value> new_node_client = v8::External::Wrap(
                (void *) r_rtmp_client);

        v8::Local<v8::Value> argv_res[] = { new_node_client, v8::String::New(
                r_rtmp_client->id) };

        data->Call(v8::Context::GetCurrent()->Global(), 2, argv_res);
    }
    else
    {
        data->Call(v8::Context::GetCurrent()->Global(), 0, NULL);
    }

    data.Dispose();
}

/**
 * 与远端服务器建立链接
 * @param args[0] ip地址
 * @param args[1] 端口号
 * @param args[2] 回调函数
 * @return
 */
Handle<Value> remote_connect(const Arguments& args)
{
    HandleScope scope;

    v8::String::Utf8Value node_ip_address(args[0]->ToString());
    std::string ip_address = std::string(*node_ip_address);
    int32_t port = args[1]->ToInt32()->Int32Value();
    Persistent<Function> temp_remote_cb;
    temp_remote_cb = Persistent<Function>::New(Local<Function>::Cast(args[2]));

    connect_remote_server(ip_address.c_str(), port, temp_remote_cb,
            _node_remote_connect_cb);

    return scope.Close(Undefined());
}

/**
 * 发送ping包
 * @param args[0] bls client
 * @return
 */
Handle<Value> send_ping_request(const Arguments& args)
{
    HandleScope scope;

    RtmpClient *client = (RtmpClient *) External::Unwrap(args[0]);

    if (NULL != client->uv_client)
    {
        RtmpProtocol *protocol = (RtmpProtocol *) client->protocol;
        protocol->send_ping_request();
    }

    return scope.Close(Undefined());
}

void init(Handle<Object> exports)
{
    exports->Set(String::NewSymbol("start_server"), FunctionTemplate::New(
            start_server)->GetFunction());

    exports->Set(String::NewSymbol("bind_read_write_buf"),
            FunctionTemplate::New(bind_read_write_buf)->GetFunction());

    exports->Set(String::NewSymbol("bind_on_msg_cb"), FunctionTemplate::New(
            bind_on_msg_cb)->GetFunction());

    exports->Set(String::NewSymbol("enable_throwup_av"), FunctionTemplate::New(
            enable_throwup_av)->GetFunction());

    exports->Set(String::NewSymbol("disable_throwup_av"), FunctionTemplate::New(
            disable_throwup_av)->GetFunction());

    exports->Set(String::NewSymbol("get_aac_sh"), FunctionTemplate::New(
            get_aac_sh_data)->GetFunction());

    exports->Set(String::NewSymbol("get_avc_sh"), FunctionTemplate::New(
            get_avc_sh_data)->GetFunction());

    exports->Set(String::NewSymbol("bind_on_close_cb"), FunctionTemplate::New(
            bind_on_close_cb)->GetFunction());

    exports->Set(String::NewSymbol("write_message_buf"), FunctionTemplate::New(
            write_message_buf)->GetFunction());

    exports->Set(String::NewSymbol("publish_stream"), FunctionTemplate::New(
            publish_stream)->GetFunction());

    exports->Set(String::NewSymbol("unpublish_stream"), FunctionTemplate::New(
            unpublish_stream)->GetFunction());

    exports->Set(String::NewSymbol("play_stream"), FunctionTemplate::New(
            play_stream)->GetFunction());

    exports->Set(String::NewSymbol("unplay_stream"), FunctionTemplate::New(
            unplay_stream)->GetFunction());

    exports->Set(String::NewSymbol("close_client"), FunctionTemplate::New(
            close_client)->GetFunction());

    exports->Set(String::NewSymbol("remote_connect"), FunctionTemplate::New(
            remote_connect)->GetFunction());

    exports->Set(String::NewSymbol("send_ping_request"), FunctionTemplate::New(
            send_ping_request)->GetFunction());

    exports->Set(String::NewSymbol("send_stream_begin"), FunctionTemplate::New(
            send_stream_begin)->GetFunction());

    exports->Set(String::NewSymbol("send_stream_eof"), FunctionTemplate::New(
            send_stream_eof)->GetFunction());

    exports->Set(String::NewSymbol("init_logger"), FunctionTemplate::New(
                init_logger)->GetFunction());

    exports->Set(String::NewSymbol("write_log"), FunctionTemplate::New(
                    write_log)->GetFunction());
}

NODE_MODULE(bls, init)
