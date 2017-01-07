{
    'targets' : [
        {
            'target_name' : 'bls',
            'include_dirs' : [
                'include',
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
                '-lpthread',
                '-lrt',
                '-lssl',
                '-lcrypto',
                '-lz',
            ],
            'cxxflags' : [
                '-O0 -g -Wall',
            ],
            'cflags!': [ '-fno-exceptions' ],
            'cflags_cc!': [ '-fno-exceptions' ],
            'cflags' : [
                '-O0 -g -Wall',
            ],
        },
    ],
}
