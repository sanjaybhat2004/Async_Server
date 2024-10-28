Functions used:

-   io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
-   io_uring_get_sqe()
        returns next available submission queue entry
        from the submission queue
-   io_uring_prep_accept() 
        Prepares an accept request, similar to usual accept()
-   io_uring_sqe_set_data()
        sets the user_data pointer field of passed sqe
-   io_uring_submit()
