{
    'targets' : [
        {
            'target_name' : 'bls',
            "engines" : { "node" : ">=0.10.3 <0.12" },
            "engine-strict" : "true",
            "engineStrict" : "true",
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
                '-O0 -g -Wall -std=c++11',
            ],
            'cflags!': [ '-fno-exceptions' ],
            'cflags_cc!': [ '-fno-exceptions' ],
            'cflags' : [
                '-O0 -g -Wall -std=c++11',
            ],
             'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                    }
                }]
            ]
        },
    ],
}
