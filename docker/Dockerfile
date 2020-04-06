FROM 42milez/cpp-dev-base:2020.4.6

ENV DEV_USER dev
ENV DEV_USER_GROUP dev

RUN useradd -ms /bin/bash dev && echo "$DEV_USER:$DEV_USER_GROUP" | chpasswd

ENV APPROOT /var/app

ENV TEST_DEFAULT_BUILD_DIR $APPROOT/cmake-build-test
ENV TEST_ASAN_BUILD_DIR $APPROOT/cmake-build-test-asan
ENV TEST_LSAN_BUILD_DIR $APPROOT/cmake-build-test-lsan
ENV TEST_MSAN_BUILD_DIR $APPROOT/cmake-build-test-msan
ENV TEST_TSAN_BUILD_DIR $APPROOT/cmake-build-test-tsan

ENV TEST_RESULT_DIR $APPROOT/test-results

RUN mkdir -p $APPROOT && chown $DEV_USER:$DEV_USER_GROUP $APPROOT && \
    mkdir -p $TEST_DEFAULT_BUILD_DIR && \
    mkdir -p $TEST_ASAN_BUILD_DIR && \
    mkdir -p $TEST_LSAN_BUILD_DIR && \
    mkdir -p $TEST_MSAN_BUILD_DIR && \
    mkdir -p $TEST_TSAN_BUILD_DIR && \
    mkdir -p $TEST_RESULT_DIR

WORKDIR $APPROOT

# see: https://docs.docker.com/engine/examples/running_ssh_service/
RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

EXPOSE 22 7777

CMD [ "/usr/sbin/sshd", "-D" ]
