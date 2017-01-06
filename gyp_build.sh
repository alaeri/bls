cur_path=`pwd`                                                                                                                                                              
sed "s/CURPATH/${cur_path//\//\\/}/g" binding.gyp.template > binding.gyp

./node/bin/node ./node/bin/node-gyp configure
./node/bin/node ./node/bin/node-gyp build
cp build/Release/bls.node node_modules/bls/bls.node

