FROM 42milez/cpp-dev-base:latest

ENV DEV_USER dev
ENV DEV_USER_GROUP dev

RUN useradd -ms /bin/bash dev && echo "$DEV_USER:$DEV_USER_GROUP" | chpasswd

ENV APPROOT /var/app
ENV TEST_BUILD_DIR $TEST_WORK_DIR/cmake-build-test
ENV TEST_RESULT_DIR $TEST_WORK_DIR/test-results

RUN mkdir -p $APPROOT && chown $DEV_USER:$DEV_USER_GROUP $APPROOT && \
    mkdir -p $TEST_BUILD_DIR && \
    mkdir -p $TEST_RESULT_DIR

WORKDIR $APPROOT

# see: https://docs.docker.com/engine/examples/running_ssh_service/
RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

EXPOSE 22 7777

CMD [ "/usr/sbin/sshd", "-D" ]
