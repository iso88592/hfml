FROM ubuntu:latest AS builder

WORKDIR /hfml

RUN apt update && apt install -y build-essential git flex bison pkg-config


RUN git clone https://github.com/qicosmos/cinatra.git /hfml/cinatra

RUN  cd /hfml/cinatra && git submodule init

COPY . /hfml/build

#RUN cd /hfml/build && make && cp /hfml/build/server /server
RUN cd /hfml/build && make

RUN cd /hfml/build/src && ./c_to_json.sh

FROM ubuntu:latest
COPY --from=builder /hfml/build/server /server
COPY --from=builder /hfml/build/src/hfml.json /export/hfml.json

ENTRYPOINT [ "/server" ]