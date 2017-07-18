#! /bin/sh
# openssh-setup.sh
# Script to verify configuration after install
#  -Conditionally creates ssh_config and sshd_config
#  -Conditionally copies host keys from /etc/ssh to /usr/local/etc/ssh
#  -Conditionally creates any non-existant host keys
#  -Renames /usr/bin/ssh, replaces with link to /usr/local/bin/ssh
cd /usr/local/etc/ssh
if [ -f ssh_config.dist -a ! -f ssh_config ]; then
  echo "Creating default /usr/local/etc/ssh/ssh_config"
  cp ssh_config.dist ssh_config
fi
if [ -f sshd_config.dist -a ! -f sshd_config ]; then
  echo "Creating default /usr/local/etc/ssh/sshd_config"
  cp sshd_config.dist sshd_config
fi

# Copy any nonexistant keys from /etc/ssh/
for key in /etc/ssh/ssh_host_*; do
  if [ ! -f /usr/local$key ]; then
    echo "Copying $key to /usr/local$key"
    cp $key /usr/local$key
  fi
done

# Then generate any missing keys
/usr/local/bin/ssh-keygen -A

# Now hide the default programs
for prog in ssh ssh-add ssh-agent ssh-keygen ssh-keyscan; do
  if [ -f /usr/bin/$prog ]; then
    echo "Hiding old program /usr/bin/$prog"
    mv /usr/bin/$prog /usr/bin/$prog.QNX
  fi
done

