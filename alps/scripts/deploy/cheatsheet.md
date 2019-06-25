### MINT configuration

~/.mint.yaml

```
arbiter_host: simpl-l4tm-580-3.u.labs.hpecorp.net:9090
arbiter_binary: /scratch/workspace/holodeck/build/src/mint/arbiter
test_path: /home/l4tm/testalps/bin/test
```

### Installing and running Librarian

```
/scratch/workspace/holodeck/bin/mica-librarian-server -v -c lfs.yaml install --repo=local
/scratch/workspace/holodeck/bin/mica-librarian-client -v -c lfs.yaml install --repo=local --no-dep
/scratch/workspace/holodeck/bin/mica-librarian-server -v -c lfs.yaml start
/scratch/workspace/holodeck/bin/mica-librarian-client -v -c lfs.yaml start 
```


### Configuring TMAS Sim-as-a-Service for use with MICA

Connect simulations together. Using the connection info for one of the nodes 
(it doesn't matter which one), issue the 'zswitch connect' command in the 
OTHER nodes control windows to connect them:

zswitch connect IP:PORT


To list connected nodes:

node_slot list

To continue execution, type c

Enable password-less root login by adding the mica ssh public key in /root/.ssh/authorized_keys



### Running Librarian on TMAS Sim-as-a-Service

TMAS comes with Librarian installed 

0. Prepare librarian configuration files

* lfs.tmas.tml

Server ip/host must match the ip address of the librarian server (this is the 
same address as the one you used to connect to the VNC session)

* l4tm_book_data.tmas.ini 

Must match the same configuration as that of the Librarian Server

1. Load cluster definition

mica-cluster create ...

2. Add user accounts and authorized keys: 

```
mica-cluster useradd tmas root --authorized_key=
mica-cluster useradd tmas l4tm --authorized_key=
```

3. Start librarian clients 





### Common Virsh commands

To list all domains:
```
virsh -c qemu:///system list --all
```

To dump domain XML:
```
virsh -c qemu:///system dumpxml <IMAGE_NAME>
```

To destroy virsh domains named *foo*:
```
for i in 0 1 2 3 ; do virsh -c qemu:///system destroy foo.vnode-$i; done; 
```

To undefine virsh domains named *foo*:
```
for i in 0 1 2 3 ; do virsh -c qemu:///system undefine foo.vnode-$i; done; rm -rf /var/tmp/holodeck/clusters/foo
```

### Diagnostics

ps aux | grep test; echo '=\='; ls -hla /lfs; echo '=\='; df -h
