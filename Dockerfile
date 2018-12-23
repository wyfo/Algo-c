FROM ubuntu:18.04

RUN apt update
RUN apt upgrade -y
RUN apt install -y vim sudo curl linux-tools-generic gcc valgrind kcachegrind
RUN apt install -y fish
RUN apt install -y clang
RUN rm -f /usr/bin/perf
RUN ln -s /usr/lib/linux-tools/4.15.0-42-generic/perf /usr/bin/perf
RUN echo 0 > /proc/sys/kernel/kptr_restrict
RUN ln -s ~/.cargo/bin/cargo /usr/bin/cargo
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
