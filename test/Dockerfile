FROM alpine:3.11.2

RUN apk update &&                                            \
    apk upgrade &&                                           \
    apk add --update --no-cache --virtual build-dependencies \
      build-base=0.5-r1                                      \
      linux-headers=4.19.36-r0 &&                            \
    apk add --update --no-cache                              \
      bash=5.0.11-r1                                         \
      cmake=3.15.5-r0                                        \
      llvm=9.0.0-r1 &&                                       \
    apk del build-dependencies
    #rm -rf /var/cache/apk/*

ENV APP_ROOT /var/app
ENV BUILD_DIR /var/app/cmake-build-test

RUN mkdir $APP_ROOT && \
    mkdir $BUILD_DIR

WORKDIR $APP_ROOT
