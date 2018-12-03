if [ -f /etc/pki/ca-trust/ca-legacy.conf ] && cmp -s /etc/defaults/etc/pki/ca-trust/ca-legacy.conf /etc/pki/ca-trust/ca-legacy.conf
then
    rm /etc/pki/ca-trust/ca-legacy.conf
fi

