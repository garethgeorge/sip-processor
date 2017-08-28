#! /bin/bash

# in dev we expect sip-processor to be mounted at /sip-processor

DEV=dev
PROD=prod

source /root/.bashrc

if [ $ENV == $PROD ]; then
  git clone https://github.com/williamberman/sip-processor
elif [ $ENV == $dev ]; then
  apt-get install tmux
fi

make -C /root/src/euca-cutils && \
make -C /root/src/mio && \
make -C /root/src/schmib_q && \
make -C /root/src/pred-duration && \
make -C /root/src/spot-predictions && \
mkdir /root/bin && \
cp /root/src/euca-cutils/ptime /root/bin/ && \
cp /root/src/euca-cutils/convert_time /root/bin/ && \
cp /root/src/schmib_q/bmbp_ts /root/bin/ && \
cp /root/src/schmib_q/bmbp_index /root/bin/ && \
cp /root/src/pred-duration/pred-distribution /root/bin/ && \
cp /root/src/pred-duration/pred-distribution-fast /root/bin/ && \
cp /root/src/spot-predictions/spot-price-aggregate /root/bin/ && \
cp /root/src/pred-duration/pred-duration /root/bin/ && \

cp -r /sip-processor/src /root/src
cp -r /sip-processor/data /mnt/data

dpg &

if [ $ENV == $PROD ]; then
  /mnt/data/runner.rb
elif [ $ENV == $dev ]; then
  tmux new-session -d "/mnt/data/runner.rb"
  tail -f /dev/null
fi
