FROM alpine:3.21

RUN apk add --no-cache g++ make valgrind clang-extra-tools gtest-dev lcov doxygen gzip

CMD ["ash"]