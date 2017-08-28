FROM golang

COPY sip-processor-entrypoint.sh /mnt/data

RUN     apt-get update && \
  curl https://bootstrap.pypa.io/get-pip.py | python && \
  pip install --upgrade --user awscli && \
  echo "export PATH=/root/.local/bin:/root/bin:$PATH" >> ~/.bashrc && \
	apt-get install -y make g++ gcc wget vim ruby && \
  go get github.com/williamberman/dpg && \
  go install github.com/williamberman/dpg && \
  go get github.com/williamberman/dpgstarter && \
  go install github.com/williamberman/dpgstarter

WORKDIR /mnt/data
ENTRYPOINT ["/mnt/data/sip-processor-entrypoint.sh"]
