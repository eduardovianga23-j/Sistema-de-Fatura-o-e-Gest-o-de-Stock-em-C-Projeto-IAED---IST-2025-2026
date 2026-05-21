FROM gcc:12 AS builder
WORKDIR /app
COPY *.c *.h ./
RUN gcc -O3 -Wall -Wextra -Werror -Wno-unused-result -o proj *.c

FROM ubuntu:22.04
WORKDIR /app
RUN apt-get update && apt-get install -y --no-install-recommends libc6 && rm -rf /var/lib/apt/lists/*
COPY --from=builder /app/proj .
CMD ["./proj"]
