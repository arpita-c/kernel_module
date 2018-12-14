### Kernel module for miscellaneous device ######

###Command#####

1. `make`
2. `sudo insmod pipe_misc_device.ko buff_len=4` 
3. `sudo chmod 666 /dev/pipe_misc_device`
4. `gcc producer_numbers.c -o producer`
5. `gcc consumer_numbers.c -o consumer`
6. `./producer /dev/pipe_misc_device`
7. `./consumer /dev/pipe_misc_device`
 
To remove the module, run:
 `sudo rmmod pipe_misc_device`
 

 
####Description#####

The program implements a kernel module of a miscellaneous character device named `pipe_misc_device`.It keeps a FIFO queue of N numbers where N is passed as module parameter.

1>Producers write numbers to /dev/pipe_misc_device.
2>Consumers read numbers from /dev/pipe_misc_device and print it.
3>when the buffer is full,there are N numbers are stored in /dev/pipe_misc_device, then any producer trying to write will block.
4>When the buffer is empty,i.e. when there are no numbers in /dev/pipe_misc_device, then any consumer trying to read will block.
5>When a consumer reads from a full pipe, it wakes up all blocked producers. In this case, no blocked consumer should be woken up.
6>When a producer writes to an empty pipe, it wakes up all blocked consumers. In this case, no blocked producer should be woken up.
7>No deadlocks. All producers and consumers make progress as long as at least one of each is running.
8>No race conditions. Each number that is written by producers is read EXACTLY once by one consumer. No number is lost. No number is read more than once. 

