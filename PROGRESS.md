### Backend Skeleton progress

Compiles and runs, only sends back "Hello world!" and closes.

1. Main done (try catch, constructs webserv and runs main loop)
2. WebServer construction done: ignoring config file for now, using only one arbitrary socket
3. WebServer loop logic done
4. ListenSocket event handler done: throw on error (server will shut down), accept on EPOLLIN
5. ClientSession event handler **not done** -> only simple logic for now, for tests

-> Step 5 is the most complicated. Receive requests, parse them, decide what to do: keep reading for body? call CGI script? construct response? Handle all errors.

Completing step 5 leads to completing CGI call logic, request parse logic, response construction logic, error handling logic.

Step 6 is to complete timeout logic (which might lead to a response).

The structure for request, request parsing, response, response constructing, cgi call, timeout logic, errors, is a work in progress.