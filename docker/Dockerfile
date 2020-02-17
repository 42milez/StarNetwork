FROM 42milez/cpp-dev-base:latest

ENV APPROOT /var/app
ENV CMAKE_BUILD_DIR $APPROOT/cmake-build-debug-remote

RUN mkdir $APPROOT && mkdir $CMAKE_BUILD_DIR

WORKDIR $APPROOT

ADD cmake/conan/conanfile.txt /root/conanfile.txt
ADD cmake/conan/profile /root/profile

RUN cd $CMAKE_BUILD_DIR && conan install /root/conanfile.txt --profile /root/profile

# see: https://docs.docker.com/engine/examples/running_ssh_service/
RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

EXPOSE 22 7777

CMD [ "/usr/sbin/sshd", "-D" ]
