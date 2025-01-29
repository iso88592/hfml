FROM ubuntu:latest

WORKDIR /hfml

RUN apt update && apt install -y build-essential git flex bison pkg-config


RUN git clone https://github.com/qicosmos/cinatra.git /hfml/cinatra

RUN  cd /hfml/cinatra && git submodule init

COPY . /hfml/build

RUN cd /hfml/build && make && cp /hfml/build/server /

ENTRYPOINT [ "/server" ]