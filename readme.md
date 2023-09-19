# Sync+Sync

A side-channel attack called Sync+Sync can be exploited within mainstream operating systems due to fsync contention. This attack has the potential to compromise the confidentiality and integrity of sensitive information.


## Covert Channel

In a Sync+Sync attack, assume the presence of two co-located entities: the receiver (attacker or sink) and the sender (victim or source), running as user-space programs concurrently. Both sides utilize files to store data.

Sync+Sync attack cases can be categorized into three classes based on their isolation environments:
1. Cross-file:
    - Sender and receiver access their respective files with exclusive access permissions.
    - These files share the same storage device, residing in either the same disk partition with the same file system (e.g., Ext4) or different partitions with the same or different file systems (e.g., Ext4 and XFS).
2. Cross-container:
    - Sender and receiver exist in two containers sharing the same device for their overlay file systems.
    - Sender and receiver can also utilize different directories with different file systems mounted on different partitions.
3. Cross-VM:
    - Sender and receiver run in separate VMs with independent disk image files.
    - Their image files must be co-located on one storage device but can reside in different partitions. In any channel configuration, both the sender and receiver can perform the following operations on their files:
    - Update file size using ftruncate + fsync.
    - Modify their own file contents using write + fsync.
    - Keep the file synchronized to the storage device using fsync-only.

The Sync+Sync channel with fsync-only incurs the shortest latency and provides the highest bandwidth for information transmission.

The sender conveys bits by either calling fsync on a file (representing bit '1') or not calling fsync (representing bit '0'). The receiver continually synchronizes the other file via fsync and measures the fsync latency to determine the values of the received bits. To transmit a bit '1', the sender synchronizes a file to submit continuous I/O requests to the disk for a predefined time period named symbol duration. Otherwise, the sender sleeps for symbol duration to transmit a bit '0'.


### How to Run

This is sample code to build Sync+Sync covert channel.

- sender: send bits via `fsync`

    If you want sender just to send `101010...`, you can remove the `#define RAND_SEND` in `sender.c`. 

    ```sh
    # channel_type: 
    #     0 -> fsync-only; 
    #     1 -> write+fsync; 
    #     2 -> ftruncate+fsync

    ./sender $sleep_time $channel_type $sender_file_path $trace_line
    ```

- receiver: revieve bits via `fsync`

    ```sh
    # channel_type: 
    #     0 -> fsync-only; 
    #     1 -> write+fsync; 
    #     2 -> ftruncate+fsync

    ./receiver $result_path $sleep_time $channel_type $receiver_file_path
    ```

## Reference

1. Qisheng Jiang and Chundong Wang. [Sync+Sync: A covert channel built on fsync with storage](https://arxiv.org/abs/2309.07657). In *the 33rd USENIX Security Symposium (USENIX Security 24)*.

