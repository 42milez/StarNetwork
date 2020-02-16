FROM alpine:3.11.2

RUN apk update                                   \
    && apk upgrade                               \
    && apk add --no-cache                        \
        bash=5.0.11-r1                           \
        build-base=0.5-r1                        \
        clang=9.0.0-r1                           \
        cmake=3.15.5-r0                          \
        git=2.24.1-r0                            \
        git-subtree=2.24.1-r0                    \
        linux-headers=4.19.36-r0                 \
        python3=3.8.1-r0                         \
    && rm -rf /var/cache/apk/*                   \
    && pip3 install --no-cache-dir --upgrade pip \
    && pip3 install --no-cache-dir conan==1.22.2

RUN mkdir /var/app

WORKDIR /var/app

CMD [ "/bin/bash" ]
