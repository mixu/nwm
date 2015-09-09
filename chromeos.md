# nwm on ChromeOS

I'll clean up these instructions once I have a chance to do another clean install. Feel free to file a PR to improve these docs meanwhile.

Crouton's capabilities for running external targets are limited, which is why you need a second, unpacked copy of crouton to install nwm at this time.

The basic steps are:

1. download and unzip Crouton into a folder by clicking the link on https://github.com/dnschneid/crouton and unzipping the file in ChromeOS.
2. add the following files: https://github.com/dnschneid/crouton/compare/master...mixu:master (view -> raw -> Save as)
3. run `sudo sh installer/main.sh -r precise -t nwm -n nwm`
4. run `sudo startnwm` (or `sh host-bin/startnwm`)

To update crouton and the chroot after a ChromeOS update, run `sudo sh installer/main.sh -f /path/to/bootstrap.tar.bz2 -r precise -t nwm -n nwm -u`.

The `-f` flag above assumes that you have a bootstrap tarball, free free to omit it if you are downloading the bootstrap on each run.
