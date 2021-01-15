1. Install Motr singlenode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You will need a CentOS 7 system to install Motr. Motr can run in both
cluster and singlenode modes. The singlenode mode can either use an available
single drive for its data storage. It can also use multiple drives if they are
available. 

The instructions on this document are tested on a VirtualBox CentOS 7 VM.
They are expected to work on real hardware and other virtualization software
such as VMWare with similar configurations.

Update your installation to specific CentOS 7 kernel and related packages.
Boot into that kernel.

>> sudo yum install kernel-3.10.0-514.6.1.el7.x86_64
>> sudo yum install kernel-devel-3.10.0-514.6.1.el7.x86_64
>> sudo yum update --exclude=kernel*
>> reboot

Install epel-release and update packages if necessary.

>> sudo yum install epel-release -y
>> sudo yum update --exclude=kernel* -y

Motr requires lustre lnet for i/o communication. Set up yum repository for
Intel Lustre 2.9 package. 

>> cat /etc/yum.repos.d/lustre-intel.repo 
[lustre-intel-2.9.0]
baseurl =
https://downloads.hpdd.intel.com/public/lustre/lustre-2.9.0/el7/client/
gpgcheck = 0
name = Intel - Lustre 2.9.0
 
Download Motr binary rpm packages.

>> ls Downloads/
motr-1.0.0-1_3.10.0_514.6.1.el7.x86_64.gitab0c26c.el7.centos.x86_64.rpm
motr-devel-1.0.0-1_3.10.0_514.6.1.el7.x86_64.gitab0c26c.el7.centos.x86_64.rpm

Make sure that kernel versions of your system matches that of the builds. Do
not attempt to install Motr if they do not.

>> uname -r
3.10.0-514.6.1.el7.x86_64

Simply yum install Motr and Motr-devel rpm packages.

>> cd Downloads
>> sudo yum install motr-1*.rpm motr-devel-1*.rpm -y

Verify installation.

>> rpm -qa | grep motr-
motr-devel-1.0.0-1_3.10.0_514.6.1.el7.x86_64.gitab0c26c.el7.centos.x86_64
motr-1.0.0-1_3.10.0_514.6.1.el7.x86_64.gitab0c26c.el7.centos.x86_64

Or.

>> yum list installed motr*
Installed Packages
motr.x86_64       1.0.0-1_3.10.0_514.6.1.el7.x86_64.gitab0c26c.el7.centos installed
motr-devel.x86_64 1.0.0-1_3.10.0_514.6.1.el7.x86_64.gitab0c26c.el7.centos installed




2. Run Motr singlenode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use the rpm command to verify that Motr is installed on your machine.

>> rpm -qa | grep motr-
motr-devel-1.0.0-1_3.10.0_514.6.1.el7.x86_64.git116e92a.el7.centos.x86_64
motr-1.0.0-1_3.10.0_514.6.1.el7.x86_64.git116e92a.el7.centos.x86_64

By default Motr runs on cluster mode and single node services are not available.
You will need to explicitly activate single node services if you plan to run Motr
on a single Virtual machine or a hardware to test its single node operations.

>> sudo m0singlenode activate
--->  Activating singlenode services
rm '/etc/systemd/system/motr-mkfs.service'
rm '/etc/systemd/system/motr.service'
rm '/etc/systemd/system/motr-client.service'
rm '/etc/systemd/system/motr-singlenode.service'
rm '/etc/systemd/system/motr-server-confd.service'
rm '/etc/systemd/system/motr-server-ha.service'
removed ‘/etc/systemd/system/motr-server@.service’

The successful execution of this command will enable all necessary single node services.
This needs to be done only once after installing a fresh copy of Motr or upgrading
an existing installation to a new version. See help for more details:

>> m0singlenode --help

Once you have successfully activated the single node Motr services you are now ready to
configure storage for Motr. For VMs and hardwares that do not have multiple disks Motr
provide a single command that creates a number of image files as Motr’s storage on their
existing hard drive. This is achieved with the following command: 
 
>> sudo m0setup -M
--->  Requested configuration: [P=8 N=2 K=1] with 1 ioservice(s), 64GiB disks in '/var/motr'
--->  Removing loop devices
--->  Removing file images
--->  Creating 8 file images, 64GiB each, in '/var/motr'
--->  Setting up loop devices
--->  Generating genders file '/etc/motr/genders'
--->  Generating disks-*.conf files
--->  Starting motr-mkfs, this may take a while...

Notice that the image files are created in the /var/motr directory.
The above setup needs to be performed only once. It erases all data and format the storage
devices for Motr.
 
Motr runs just like any other Linux daemons. Use the following commands to start, stop,
restart and check the status of Motr. See commands and example outputs below.

Start:
>> sudo m0singlenode start
--->  Starting motr-singlenode services
--->    waiting for lnet                       [  OK  ]
--->    waiting for motr-kernel  .             [  OK  ]
--->    waiting for motr-server-ha             [  OK  ]
--->    waiting for motr-server-confd          [  OK  ]
--->    waiting for motr-server@mds  .         [  OK  ]
--->    waiting for motr-server@ios1           [  OK  ]
--->    waiting for motr                       [  OK  ]
--->    waiting for motr-client                [  OK  ]
--->    waiting for motr-singlenode            [  OK  ]

Stop:
>> sudo m0singlenode stop


➤ Please ignore the HA service failed error messages when stopping Motr. This is a known
issue in Singlenode Motr. HA service stops properly on cluster mode.

Restart:
>> sudo m0singlenode restart

Status:
>> sudo m0singlenode status
### Global state --------------------------------
 State:                  loaded
 Autostart:              disabled
 LNet:                   active    Fri 2016-11-18 11:08:26 GMT
 LNet Network ID:        10.0.2.15@tcp

### Kernel --------------------------------------
 motr-kernel             active    Fri 2016-11-18 11:11:02 GMT
   m0ctl                  44181  1 
   m0motr               4238578  6 m0ctl
   m0gf                  121518  1 m0motr
   lnet                  351271  3 m0motr,ksocklnd
   libcfs                521557  3 lnet,m0motr,ksocklnd

### Servers -------------------------------------
 motr-server-ha          active    Fri 2016-11-18 11:11:02 GMT    PID:7786
 motr-server-confd       active    Fri 2016-11-18 11:11:02 GMT    PID:7960
 motr-server@mds         active    Fri 2016-11-18 11:11:03 GMT    PID:8145
 motr-server@ios1        active    Fri 2016-11-18 11:11:04 GMT    PID:8371

### Trace ---------------------------------------
 motr-trace@kernel       active    Fri 2016-11-18 11:11:02 GMT
 motr-trace@ha           active    Fri 2016-11-18 11:11:02 GMT
 motr-trace@confd        active    Fri 2016-11-18 11:11:02 GMT
 motr-trace@mds          active    Fri 2016-11-18 11:11:03 GMT
 motr-trace@ios1         active    Fri 2016-11-18 11:11:04 GMT

The above commands can be executed as often as you like to start, stop,
restart and check the status of Motr service on the system.


You can use the m0t1fs interface to quickly perform some i/o in Motr. Use the following
commands to create a single file and store it in the Motr object store.

>> sudo touch /mnt/m0t1fs/0:3000
>> sudo setfattr -n lid -v 8 /mnt/m0t1fs/0:3000
>> sudo dd if=/dev/zero of=/mnt/m0t1fs/0:3000 bs=4M count=10
10+0 records in
10+0 records out
41943040 bytes (42 MB) copied, 5.12327 s, 8.2 MB/s

The setfattr command can be used to set the block size of the object. In the above example
the value 8 corresponds to 512KB block size. The following table lists all possible block
sizes available in m0t1fs:

Value Size
0     NA
1     4KB
2     8KB
3     16KB
4     32KB
5     64KB
6     128KB
7     256KB
8     512KB
9     1MB
10    2MB
11    4MB
12    8MB
13    16MB
14    32MB

Once you have created a file you can verify its existence by using the the 'ls'
command in Linux. However notice that the ls command's system call implementation
is currently limited that it will not list all the contents of the mounted
directory /mnt/m0t1fs. But 'ls -lh' on a specific file works. See example below:

>> sudo ls -lh /mnt/m0t1fs/0:3000

Or, as shown below use a for loop to create a number of files in one go:

>> sudo su
>> for i in {0..7}; do file=/mnt/m0t1fs/0:300$i; rm -f $file; done;
>> for i in {0..7}; do file=/mnt/m0t1fs/0:300$i; touch $file; setfattr -n lid -v 8 $file; dd if=/dev/sda of=$file bs=2M count=10; done;
>> for i in {0..7}; do file=/mnt/m0t1fs/0:300$i; ls -lh $file; done;
>> exit

Notice that the only POSIX like interface currently we have to interact with Motr
is m0t1fs which is available from /mnt/m0t1fs directory of the system. This
interface currently allows you to read/write/create files in Motr.

However there is one limitation. You will need to manage filenames (metadata).
You can use filenames which have the format <int>:<int> such as 0:5000 for example.
The numbers in the names simply refer to the Motr object identifiers. So, unless
your applications are willing to use (and manage) this kind of file names it will
be difficult to run unmodified applications on this interface at this stage.

In fact the file names can be specified more generally using two hexadecimal numbers
separated by a colon. The following are all valid file names in m0t1fs:
0:100, 20:30, abc:def, 0x1a39:35, 0xfa0202002:0x3838ab, etc. 
 
In other words, applications can create files with <int>:<int> or <hex>:<hex> name format.
The files will be there. Applications can read and write but commands like 'ls -la'
will not list the file as illustrated by dd command example above.

Execute the following command to terminate i/o via m0t1fs:

>> sudo umount /mnt/m0t1fs/

You will need to properly shutdown the Motr server before turning off the server.

>> sudo m0singlenode stop
>> shutdown [-h/-r] now

When you power on the server execute the following the commands to safely start the motr server.

>> sudo m0setup -E         # instruct motr to use existing storage devices
>> sudo m0singlenode start # start motr



3. Connecint to Motr singlenode with Motr Client
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Motr Client is the client interface to connect to Motr and access its data and
metadata.

You need to compile and build your application with motr-devel rpm installed.
After successfully build the Motr Client application, you can run it from the
same node running Motr or from another node.

Let's suppose Motr is running on NodeA (in singlenode mode), with NodeA_IP as
its IP address. Motr Client application is about to run on NodeB, and its IP
address as NodeB_IP. Then, the following parameter should be provided to Motr
Client application:

    "NodeB_IP@tcp:12345:33:103 NodeA_IP@tcp:12345:34:1 <0x7000000000000001:0> <0x7200000000000001:64>"

Please enjoy.


4. Download RPMs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

motr:       https://drive.google.com/open?id=1dotqCSojhA_NJkQVLKsR-CbgIHHRYI63
motr-devel: https://drive.google.com/open?id=1raLn-K1BULx2zLUoFiPyXNc3GJb9ft6F

If you have a different kernel version, please let me know, and I will build
these two packages for the specified kernel.
