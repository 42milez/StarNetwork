FROM 42milez/cpp-dev-base:latest

ENV APPROOT /var/app

RUN mkdir $APPROOT

WORKDIR $APPROOT

ENV CONAN_PROFILE=/root/.conan/profiles/default

# see: https://docs.docker.com/engine/examples/running_ssh_service/
RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

EXPOSE 22 7777

CMD [ "/usr/sbin/sshd", "-D" ]
