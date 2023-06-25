# The Event Loop and Nonblocking IO

There are 3 ways to deal with concurrent connections in server-side network programming:
- forking
- multi-threading
- event loops


```
// Simplified pseudo-code for the event loop

add_fds = [...]
while True:
    active_fds = poll(all_fds)
    for each fd in active_fds:
        do_something_with(fd)

def do_something_with(fd):
    if fd is a listening socket:
        add_new_client(fd)
    elif fd is a client connection:
        while work_not_done(fd):
            do_something_to_client(fd)

def do_something_to_client(fd):
    if should_read_from(fd):
        data = read_until_EAGAIN(fd)
        process_incoming_data(data)
    while should_write_to(fd):
        write_until_EGAIN(fd)
    if should_close(fd):
        destroy_client(fd)
```

We use `poll` operation to tell us which fd can be operated immediately without blocking.

When we perform an IO operation on an fd, the operation should be performed in the nonblocking mode.

In blocking mode,

- `read` blocks the caller when there are no data in the kernel
- `write` blocks when the write buffer is full
- `accept` blocks when there are no new connections in the kernel queue

In nonblocking mode, these operations either

- `success` without blocking
- `fail` with the errno `EAGAIN`, meaning "not ready"

Nonblocking operations that fail with `EAGAIN` must be retried 
after the readiness was notifed by the `poll`.

The `poll` is the sole blocking operation in an event loop, 
everything else should be nonblocking;
thus, a single thread can handle multiple concurrent connections.

All blocking networking IO APIs, such as `read`, `write`, and `accept`, have a nonblocking mode.

APIs that do not have a nonblocking mode, such as `gethostbyname`, and disk IOs, should be performed in thread pools.

Also, timers must be implemented within the event loop since we can't sleep waiting inside the event loop.

```
// Setting an fd to nonblocking mode
static void fd_set_nb(int fd) {
    errno = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if (errno) {
        die("fcntl error");
        return;
    }

    flags |= O_NONBLOCK;

    errno = 0;
    (void)fcntl(fd, F_SETFL, flags);
    if (errno) {
        die("fcntl error");
    }
}
```

On Linux, besides the `poll` syscall, there are also `select` and `epoll`.

The ancient `select` syscall is basically the same as the `poll`,
except that the maximum fd number is limited to a small number,
which makes it obsolete in modern applications.

The `epoll` API consists of 3 syscalls: `epoll_create`, `epoll_wait`, and `epoll_ctl`.

The `epoll` API is stateful, instead of supplying a set of fds as a syscall argument,
`epoll_ctl` was used to manipulate an fd set created by `epoll_create`,
which the `epoll_wait` is operating on.
