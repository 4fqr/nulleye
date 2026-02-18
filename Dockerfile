FROM ubuntu:22.04
RUN apt-get update && apt-get install -y build-essential clang llvm libelf-dev libbpf-dev bpftool libncurses-dev libssl-dev libyaml-dev libsqlite3-dev libpcap-dev libmaxminddb-dev python3 python3-venv python3-dev git wget pkg-config
WORKDIR /work
COPY . /work
RUN make -j"$(nproc)"
CMD ["/work/nulleye"]
