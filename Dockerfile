FROM ubuntu:20.04

RUN apt-get update -y
RUN apt-get install gcc-multilib -y

RUN mkdir /leforum
RUN mkdir -p /leforum/.leforum_server
ADD build/LeForumServer /leforum
ADD run_server.sh /leforum

RUN useradd -rm -d "/leforum" -s /bin/bash -u 7431 "uleforum"

RUN chown uleforum:uleforum /leforum
RUN chown -R uleforum:uleforum /leforum/

RUN chmod 755 /leforum
RUN chmod 711 /leforum/LeForumServer
RUN chmod 711 /leforum/run_server.sh

WORKDIR /leforum
USER uleforum

EXPOSE 7431
CMD ["./run_server.sh"]
