FROM alpine:3.11.2

RUN apk update &&               \
    apk upgrade &&              \
    apk add --update --no-cache \
      bash=5.0.11-r1            \
      build-base=0.5-r1         \
      clang=9.0.0-r1            \
      cmake=3.15.5-r0           \
      g++=9.2.0-r3              \
      gcc=9.2.0-r3              \
      linux-headers=4.19.36-r0  \
      make=4.2.1-r2
    #rm -rf /var/cache/apk/*

ENV APP_ROOT /var/app
ENV BUILD_DIR /var/app/build

RUN mkdir $APP_ROOT && \
    mkdir $BUILD_DIR

WORKDIR $APP_ROOT
