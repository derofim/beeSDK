#include "bee_connection.h"
#include "base/common.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <tchar.h>

#ifdef LEAK_CHECK
#include <vld.h>
#endif

BeeLocalServer::Ptr svr;

typedef struct ChannelInfo {
    SOCKET sock;
    bee_handle handle;
    std::string user_agent;
    std::string stream_name;

    ChannelInfo() {
        sock = NULL;
        handle = -1;
    }
}ChannelInfo;

//////////////////////////////////callbacks////////////////////////////////////////
void BEE_CALLBACK bee_sys_cb(bee_handle handle, BeeErrorCode ec1, bee_int32_t ec2, const char *message, void *opaque) {
    //Not implemented.
}

void BEE_CALLBACK bee_play_cb(bee_handle handle, BeeErrorCode ec1, bee_int32_t ec2, const char *message, void *opaque) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    ChannelInfo *channel = NULL;
    bool bret = true;
    BeeConnection::Ptr connection;
    do 
    {
        if (opaque == NULL) {
            bret = false;
            break;
        }

        channel = (ChannelInfo *)opaque;
        printf("[%s][%d] Play a session:%d, ec1:%d, ec2:%d, msg:%s\n", channel->user_agent.c_str(), channel->sock, handle, ec1, ec2,message==NULL?"NULL":message);

        if (svr == NULL) {
            bret = false;
            break;
        }

        connection = std::dynamic_pointer_cast<BeeConnection>(svr->get_connection(channel->sock));
        if (ec1 != kBeeErrorCode_Success) {
            bret = false;
            break;
        }
    } while (0);

    if (channel != NULL) {
        delete channel;
    }

    if (connection != NULL) {
        connection->on_play(bret, handle);
    }
}

void BEE_CALLBACK bee_open_cb(bee_handle handle, BeeErrorCode ec1, void *opaque) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    ChannelInfo *channel = NULL;
    bool bret = true;
    do 
    {
        if (opaque == NULL) {
            bret = false;
            break;
        }

        channel = (ChannelInfo *)opaque;
        channel->handle = handle;

        printf("[%s][%d] Open a session:%d, ec1:%d\n", channel->user_agent.c_str(), channel->sock, handle, ec1);

        if (channel->handle == -1 || ec1 != kBeeErrorCode_Success) {
            bret = false;
            break;
        }               

        std::ostringstream os;
        os << "{\"stream_name\"" << ":\"" << channel->stream_name << "\"}";
        std::string args = os.str();
        bee_int32_t timeout = -1;
        ret = bee_async_play(handle, args.c_str(), args.size(), timeout, bee_play_cb, opaque);
        if (ret != kBeeErrorCode_Success) {
            bret = false;
            break;
        }        
    } while (0);

    if (!bret) {
        if (channel != NULL) {
            if (svr != NULL) {
                BeeConnection::Ptr connection = std::dynamic_pointer_cast<BeeConnection>(svr->get_connection(channel->sock));
                connection->on_play(false, handle);
            }
            delete channel;
        }
    }
}

void BEE_CALLBACK bee_read_cb(bee_handle handle, const bee_uint8_t *data, bee_uint32_t len, bool eof, void *opaque) {
    if (svr != NULL) {
        SOCKET sock = (SOCKET)opaque;
        BeeConnection::Ptr connection = std::dynamic_pointer_cast<BeeConnection>(svr->get_connection(sock));
        if (connection != NULL) {
            connection->on_read(handle, data, len, eof);
        }
    }
}

void BEE_CALLBACK bee_init_cb(BeeErrorCode ec1, bee_int32_t ec2, void *opaque) {
    printf("init result ec1:%d,ec2:%d\n", ec1, ec2);
}

bool TEST_CALLBACK play_bee_stream(const std::string &stream_name, const std::string &user_agent, bee_handle &handle, void *opaque) {
    bool bret = true;
    BeeErrorCode ret = kBeeErrorCode_Success;
    ChannelInfo *channel = NULL;
    do {
        channel              = new ChannelInfo;
        channel->sock        = (SOCKET)opaque;
        channel->user_agent  = user_agent;
        channel->stream_name = stream_name;

        printf("[%s][%d] Receive play request stream:%s\n", user_agent.c_str(), channel->sock, stream_name.c_str());

        ret = bee_async_open(bee_open_cb, (void*)channel);
        if (ret != kBeeErrorCode_Success) {
            bret = false;
            break;
        }
    } while (0);
    if (!bret && channel != NULL) {
        delete channel;
    }
    return bret;
}

void TEST_CALLBACK stop_bee_stream(bee_handle handle) {
    bee_close(handle);
}

BeeErrorCode TEST_CALLBACK read_bee_stream(bee_handle handle, bee_int64_t offset, bee_int32_t len, bee_int32_t timeout, void *opaque) {
    return bee_async_read(handle, offset, len, timeout, bee_read_cb, opaque);
}

void init_param(BeeSystemParam &param, BeeSystemCallbacks &callback) {
    memset(&param, 0, sizeof(BeeSystemParam));
    memset(&callback, 0, sizeof(BeeSystemCallbacks));

    param.platform_type         = kPlatformType_PC;
    param.net_type              = kNetType_WireLine;
    param.app_name              = "test";
    param.app_version           = "1.0";
    param.system_info           = "win10";
    param.machine_code          = "1234567890";
    param.log_path              = ".";
    param.log_level             = kLogLevel_Debug;
    param.session_count         = 16;
    
    callback.system_notify      = bee_sys_cb;
    callback.init_callback      = bee_init_cb;
}

BeeErrorCode init_bee() {
    BeeSystemParam param;
    BeeSystemCallbacks sys_cb;
    init_param(param, sys_cb);

    bee_int32_t ec2 = 0;
    bee_int32_t timeout = 2000;
    BeeErrorCode ec1 = bee_sync_init(&param, &sys_cb, timeout, &ec2);
    return ec1;
}

BeeErrorCode test_sync_mode() {
    BeeErrorCode ec1 = init_bee();
    do 
    {
        std::string stream_name = "ABCD";
        bee_handle handle = bee_sync_open(&ec1);
        if (handle == -1 || ec1 != kBeeErrorCode_Success) {
            break;
        }

        std::ostringstream os;
        os << "{\"stream_name\"" << ":\"" << stream_name << "\"}";
        std::string args = os.str();
        ec1 = bee_sync_play(handle, args.c_str(), args.size(), -1);
        if (ec1 != kBeeErrorCode_Success) {
            break;
        }

        FILE *fp = fopen("sync.mp4", "wb+");            
        size_t count = 1000;
        size_t size = 32*1024;
        bee_uint8_t *buff = new bee_uint8_t[size];
        while (count--) {
            size_t len = bee_sync_read(handle, buff, size, -1);
            if (len == 0) {
                break;
            }

            fwrite(buff, len, 1, fp);
            printf("%d Write %d bytes\n", count, len);
        }
        delete [] buff;
        fclose(fp);
        bee_close(handle);
        printf("Done\n");
    } while (0);
    return ec1;
}

void BEE_CALLBACK VideoConfCallback(bee_handle handle, const char *data, bee_uint32_t size, void *opaque) {
    //printf("@@@ [handle=%d][conf=%I64d][id=%I64d][display=%s][type=%d][ec=%d]\n", handle, conf_id, party_id, party_display, party_type, ec1);
}

int main(int argc, char **argv) {
    BeeErrorCode ec1 = kBeeErrorCode_Success;
    do {
        ec1 = init_bee();
        if (kBeeErrorCode_Success != ec1) {
            printf("Initialize failed with %d\n", ec1);
            break;
        }        
    } while (0);

    bool testing_live = true;
    bool testing_conf = false;

    if (ec1 == kBeeErrorCode_Success) {
        if (testing_conf) {
            bee_handle session = bee_sync_open(&ec1);
            if (session != -1) {
                const char *args = "{\"room_name\":1234,\"my_name\":\"hezhen\"}";
                size_t args_len = strlen(args);
                ec1 = bee_join_video_conference(session, args, args_len, 5000, VideoConfCallback, NULL, NULL, NULL);
            } else {
                printf("bee_sync_open return %d\n", ec1);
            }
        }

        if (testing_live) {
            bool start_svr = true;
            do {
                if (argc != 2) {
                    break;
                }

                int start_server = atoi(argv[1]);
                if (start_server == 0) {
                    start_svr = false;
                }
                else {
                    start_svr = true;
                }
            } while (0);

            if (start_svr) {
                std::string listen_ip = "0.0.0.0";
                uint16_t listen_port = 8888;

                PlayBeeCallbacks callbacks;
                callbacks.play = play_bee_stream;
                callbacks.read = read_bee_stream;
                callbacks.stop = stop_bee_stream;
                svr.reset(new BeeLocalServer);
                if (!svr->start(listen_ip, listen_port, (void*)&callbacks)) {
                    printf("Local server fail to listen on %s:%d.\n", listen_ip.c_str(), listen_port);
                }
                else {
                    printf("Local server listen on %s:%d success.\n", listen_ip.c_str(), listen_port);
                }
            }
            else {
                ec1 = test_sync_mode();
            }
        }
    }

    printf("Press to exit.\n");
    getchar();
    
    bee_uninit();

    if (svr != NULL) {
        svr->stop();
        svr.reset();
    }
    return 0;
}
