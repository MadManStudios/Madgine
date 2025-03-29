FROM  ubuntu:latest

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y dos2unix python3 build-essential git pkg-config procps \
            libbz2-dev libffi-dev libgdbm-dev libgdbm-compat-dev liblzma-dev \
            libncurses5-dev libreadline6-dev libsqlite3-dev \
            lzma lzma-dev tk-dev uuid-dev zlib1g-dev

COPY entrypoint.sh /entrypoint.sh

RUN dos2unix /entrypoint.sh

RUN git clone https://github.com/emscripten-core/emsdk.git \
&& cd emsdk \
&& ./emsdk install latest \
&& ./emsdk activate latest

CMD ["/entrypoint.sh"]
#CMD ["/bin/bash"]