cd ./build
###
# @Author: chiyuan shaome@foxmail.com
# @Date: 2023-06-15 17:56:09
 # @LastEditors: chiyuan shaome@foxmail.com
 # @LastEditTime: 2023-07-10 13:54:35
# @FilePath: /jincubator/create.sh
# @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
###
if [[ $1 == "debug" ]]; then
    rm -rf *
    rm -rf ../jincubator ../jincubator.ini
    cmake ..
    make
    cp ./src/jincubator ../
    cp ../src/jincubator.ini ../
    # ssh root@120.48.86.153 -p 10029  "pkill -f ./jincubator"
    # scp  ./src/jincubator root@192.168.3.174:/root/tmp
    # scp ./src/jincubator root@10.64.53.52:/root/work/io
fi

if [[ $1 == "build" ]]; then
    make
    cp ./src/jincubator ../
    cp ../src/jincubator.ini ../
    # ssh root@120.48.86.153 -p 10137  "pkill -f ./jincubator"
    # scp -P 10137 ./src/jincubator root@120.48.86.153:/root/work/io
    scp ./src/jincubator root@100.77.130.24:/root/work/io
fi

if [[ $1 == "cp" ]]; then
    # rm -rf *
    # cmake ..
    # make    
    # ssh root@120.48.86.153 -p 10029  "pkill -f ./jincubator"
    scp ./src/jincubator root@192.168.3.174:/root/work/io
fi

if [[ $1 == "scp" ]]; then
    rm -rf *
    cmake ..
    make
    # scp  ./src/jincubator   root@192.168.3.127:/root/jcore/
    scp  ./src/jincubator   root@192.168.3.127:/root/work/io
    # scp -P 9022  ./src/jincubator  root@120.48.86.153:/root/jcore/
fi
