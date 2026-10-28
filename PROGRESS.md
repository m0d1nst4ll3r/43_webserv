### Backend Skeleton progress

1. Main done (try catch, constructs webserv and runs main loop)
2. WebServer construction done: ignoring config file for now, using only one arbitrary socket
3. WebServer loop logic done
4. ListenSocket event handler done: throw on error (server will shut down), accept on EPOLLIN
5. ClientSession event handler **not done** -> need to write simple test logic