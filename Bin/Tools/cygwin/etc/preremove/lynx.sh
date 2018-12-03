if [ -f /etc/lynx-site.cfg ] && cmp -s /etc/defaults/etc/lynx-site.cfg /etc/lynx-site.cfg
then
    rm /etc/lynx-site.cfg
fi

