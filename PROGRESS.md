### Program progress

Compiles and runs, parses config, only sends back "Hello world!" and closes.

Steps completed (may be bugged, or imperfect):

1. Main done (try catch, constructs webserv and runs main loop)
2. Config parsing done
3. WebServer construction done: uses config, constructs many listen sockets, passes config around
4. WebServer loop logic done
5. ListenSocket event handler done: throw on error (server will shut down), accept on EPOLLIN
6. ClientSession event handler **not done** -> only simple logic for now, for tests

### To-do

Next steps:

- Build request parser to parse requests
- Build error system to construct error responses (errors can happen at many points: request receiving, parsing, verifying against website config, ...)
- Build response constructor
- Build CGI call logic
- Build timeout logic (for ClientSession and CGICall)

For later (don't worry about this yet):

- Test and bug-hunt (e.g config parsing, request parsing, DDoS, slow-loris, try to make server crash)
- Find better name for IOChannel
- Maybe rename Config to WebsiteConfig to better differentiate from ServerConfig
- Maybe use ServerConfig in Config parsing for some default values/caps