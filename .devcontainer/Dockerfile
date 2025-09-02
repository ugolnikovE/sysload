FROM debian:stable-slim

RUN apt-get update \ 
    && apt-get install -y git gcc g++ make cmake gdb \ 
    && apt-get clean

WORKDIR /root

CMD ["sleep infinity"]
