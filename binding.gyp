{
    'targets' : [
        {
            'target_name' : 'bls',
            'include_dirs' : [
                'include',
                'include/comlog',
            ],
            'defines' : [
            'NODE_EXTEND',
            ],
            'sources' : [
                'src/addon.cpp',
                'src/BlsHandShake.cpp',
                'src/BlsLogger.cpp',
                'src/BlsMessage.cpp',
                'src/BlsSocket.cpp',
                'src/RtmpChunkPool.cpp',
                'src/RtmpClient.cpp',
                'src/RtmpProtocol.cpp',
                'src/RtmpServer.cpp',
                'src/utilities.cpp',
                'src/BlsConsumer.cpp',
                'src/BlsSource.cpp',
            ],
            'libraries' : [
                './lib/libullib.a',
                '-lpthread',
                '-lrt',
                '-lssl',
                '-lcrypto',
                '-lz',
            ],
            'cxxflags' : [
                '-O0 -g -Wall',
            ],
            'cflags' : [
                '-O0 -g -Wall',
            ],
        },
    ],
}
