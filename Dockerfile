FROM golang

COPY src  /root/src

COPY data /mnt/data    

COPY sip-processor-entrypoint.sh /mnt/data

RUN     apt-get update && \
  curl https://bootstrap.pypa.io/get-pip.py | python && \
  pip install --upgrade --user awscli && \
  echo "export PATH=/root/.local/bin:/root/bin:$PATH" >> ~/.bashrc && \
	apt-get install -y make g++ gcc wget vim ruby && \
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
  go get github.com/williamberman/dpg && \
  go install github.com/williamberman/dpg && \
  go get github.com/williamberman/dpgstarter && \
  go install github.com/williamberman/dpgstarter

WORKDIR /mnt/data
ENTRYPOINT ["/mnt/data/sip-processor-entrypoint.sh"]
