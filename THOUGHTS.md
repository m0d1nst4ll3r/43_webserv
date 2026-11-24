Request parsing thoughts:

- Why ReceiveStatus and ReceiveResult?

When calling recv, we have had an EPOLLIN just before. The recv call can either:
a. Succeed with >= 1 return (read at least one byte)
b. Fail with -1 return
c. "Succeed" with 0 return

When it succeeds, we read the bytes and jump to next state if needed.

When it fails with -1, we want to just close the connection, because some unexpected error happened on the socket. We don't even build a response, because the socket is compromised, just close and that's it.

When it "succeeds" with 0, it just means there was nothing to read. Why? We just received EPOLLIN, clearly there WAS something to read. This is also an unexpected error, so we will also just close the socket and that's it.

The socket recv() function should just notify of an error (-1 or 0), it doesn't need to discriminate.

- An additional note on recv() return

A -1 return is an error. We cannot check errno, so we cannot know what type, if recoverable or not. So we should still close no matter what.

A 0 return, however, is NOT an error. It is simply an indication that the client has closed its write-end. It is an indication that keeping the connection alive after sending all the data is useless, because the client will never send anything back.

It is therefore interesting for recv() to notify the caller that 0 was received. The simplest way to do this is to return an empty string.

There should still be no need to clutter the code with a special structure anyway.

- char* instead of string

We could be using char* instead of string where null bytes are possible. I started out by using string for simplicity, but we will have to read null bytes, because we will have to read binary.

I don't truly know whether it is best practice to use string or char* for data containing null bytes. Technically, string can handle it perfectly. E.g `std::string data("AB\0C", 4)`.

- Null bytes (continuation of char*)

Where are null bytes used anyway?

Null bytes are found only in the body of HTTP messages, apparently.

We can only be using char*, as we are right now, for fixed length buffers for recv().

- HTTPRequest class and construction

The HTTPRequest class is there to hold all of a Request. It should have:

The method
The target
The protocol
The headers (in a map)
The body

Right now I haven't added body yet because this is WIP. But I don't see it in the WIP branch. Where is body stored?

By the way, method, target, protocol, headers can all be strings, but body needs to be char*, because it is a string of bytes, not a string of readable characters. These bytes might be null.

How should it be constructed? And when?

Basically the Client Session will recv() until finding a \r\n\r\n or \n\n (for compatibility, although not strictly needed). It will then process everything and either fail with a bad request, or move on to processing a valid request, or move on to reading the body, if any.

This parsing/verification process can happen directly inside the construction of a request. So the request constructor should be called right after receiving \r\n\r\n, and it should do some basic verifications itself, notably:

Is the method valid at all, does the server accept it (note that whether or not the server accepts it can happen after).
Is the protocol valid at all, does the server accept it (same thing).
Is everything else valid? Is the formatting valid?

Some stuff that we can verify:

Is the first line too long?
Is any header line too lengthy?

Note that whether the entire thing is too long or not will be caught right after a recv() and we won't even try to build a request, we will just abort and respond 

- Request Parsing Details

General syntax checks

Header Line: `name:(OWS)value(OWS)\r\n` means no whitespace before or after header name, but whitespaces (tabs, spaces) possible before and after value
Header Name: a-z A-Z 0-9 and !#$%&'*+-.^_`|~ however header names are transformed into lowercase, also cannot be empty
Header Value: can be empty, and can contain any byte except 7F (del) and anything from 00 to 1F (before space), except tab (09), this means from 80 to ff is allowed, and from 20 to 7E is allowed, and 09 is allowed. (OWS) is trimmed at edges of value.
Method: like header name, cannot be empty, yes this means get_03 is valid, this is case-sensitive meaning get and GET are not the same
Protocol: HTTP/X.X exactly, X is replaced by one digit
Target: a-z A-Z 0-9 `-._~` non-reserved characters and `:/?#[]@!$&'()*+,;=` reserved characters, these are all allowed, though any character outside of that is allowed by encoding it with %hh

Note that for target (the most important), a % has to be followed by 2 digits. Literal spaces are not allowed, non ascii characters should be percent encoded (like é)

Specific header value semantics

content-length obviously must only contain digits
connection has specific values like "close" or "keep-alive"

- Target parsing

`/path/to/resource?name=hello%20world`

Whatever follows `?` is a query. In this case the server needs to separate `/path/to/resource` from `?name=hello%20world`. The former is the file path, the latter is a query. The server should not look for a file named `resource?name=hello%20world`.

In the case that the resource is a static file, the server _ignores_ the query. However, in the case that the resource is _dynamic_ (handled by application code or a script), we pass the raw query string to it (here, `name=hello%20world`). The application should decode the string itself.

In our case, the query is only useful in a CGI script. But on regular servers, they can be useful for much more (don't ask me what).

`/path/to/myfile.html?key1=value1&key2=value2#SomewhereInTheDocument`

Whatever follows `#` is a fragment. This is used by the client (browser, probably) to set an anchor in the resulting page. This works with markdown files, for example.

The server will never receive this. Clients do not send fragments as part of the request, they just use it for themselves. `#` is not a valid unencoded URI character.

- Legal Protocol, Legal Method

For now I set it up so that we can, directly at parse, say that `ASDF` is not a recognized method anywhere and so we should not even return a 501 error, but rather 400. This is overkill for our scope, not to mention that technically `ASDF` *could* be a valid method somewhere because the HTTP specification does not enforce any method, but rather allows methods to be extended per user need. The way some web servers do it is that they have a list of hardcoded methods that are valid. However, some others do not. Honestly, for our case, the simplest is to just say we do not handle `ASDF` rather than say `ASDF` is not a valid method. This eliminates the need for "legal method" in ServerConfig.

Similarly, HTTP/9.9 is not a protocol that exists anywhere, but the HTTP spec says HTTP/X.X is a valid protocol format. You might be confused about HTTP/2 and HTTP/3 but these protocols don't even use the same protocol framing at all, so don't even bother thinking about them. For our basic usecase, HTTP/X.X is the framing, and HTTP/1.2 might exist, but we just don't handle it. At least, I think this makes much more sense. This also eliminates the need for "legal protocol" in ServerConfig.

My recommendation is that we just don't send anything to the Request constructor other than the string.

- \r\n vs \n

On this question, modern web servers like nginx and apache accept \n. They use tolerant parsing. However, this can make the server vulnerable to request smuggling attacks if the request goes through several parsers that might be differently configured. Security-conscious engines do not tolerate bare \n and may even reject it outright instead of ignoring it.

For our case, either is fine. Tolerating \n is convenient for our own tests.

- ALL of the ERRORS

In order that we might catch the errors.

None: If the socket has any issue and we can't respond
429: Too many requests (we will need to implement this!)
400: Malformed input, method is invalid, or too long, or protocol is invalid, or too long, or received bytes are too much (before body)
414: Target field is too long
501: Server does not handle this method (might want to bundle invalid methods in there like `ASDF`)
505: Server does not handle this protocol (protocol should be parsed like HTTP/X.X)
431: Header line is too long (a single line)

405: Website config does not allow this method on target resource
404: Can't find resource


Headers present in errors:
`Connection: close` included when we stopped reading the TCP stream mid-header to preserve memory. Connection sync is lost so we cannot resume.
`Content-type` with something like `text/html` if an error page is included, with its `Content-length`.
`Date` and `Server` metadata headers to identify who rejected the request (because it might not be our server, it might be something in-between).
`Retry-After` with how many seconds to wait. Useful for some errors like 429 (rate limiting).
`Allow` followed with allowed methods like `GET, POST`, this is used for 405 errors.

- HTTP protocols?

HTTP/0.9 HTTP/1.0 HTTP/1.1

The subject suggests 1.0 only.

However, the subject also mentions chunked encoding, which is a 1.1 requirement. This is probably an error: the subject previously required 1.1, but was changed to 1.0. They probably forgot to remove the line about chunked encoding, or decided to leave it for whoever decides to go for 1.1.

0.9 should be ignored: it is obsolete and only required for servers that want to be backwards compatible for some reason. We don't, and handling it complicates things.

We could simply go for 1.0 as suggested by the subject. Connections are always closed after response in 1.0 and there is no chunked encoding.

We can also go for 1.1. This is what I blindly went for before even deciding anything, because it just made sense, but if only 1.0 is needed, then we can backtrack and redesign.

1.1 adds chunked encoding, enforces the host header for virtual hosting (which we might want, or might not want to implement). For now we don't implemented virtual hosting. 1.1 also adds a "expect: 100-continue" header if the client wants to send the body separately, which complicates things further. If we don't implement it we should at least handle it gracefully. It also adds different request-target formats like `http://example.com/path` instead of `/example.com/path` - we can totally decide to reject that format.

Note: `Connection: keep-alive` is an opt-in for HTTP/1.0, we should have a special case for it to override the default 1.0 close behaviour. Or we can ignore it. This is an unofficial extension.
Note: HTTP/1.2 is handled by *some* servers by using the highest version of HTTP 1 that they support, so 1.1 - this is recommended by RFC 9112, but many modern servers will choose to strictly reject the request.

### Thoughts Prompt

I am working on my c++98 web server school project. My teammate completed the config parser and I completed the back end skeleton. I know that the program structure I implemented is not perfect or ideal, but this is a learning experience, as this is my first real C++ project, so it is good enough for me now. We're now moving on to the details of the back end implementation: request parsing, response construction, CGI scripts, timeout logic, and all of the error cases which will cause a client session to drop after potentially sending an error response.

Let me describe the structure first. The program's heart is a WebServer class, which is my orchestrator - it sets up signal management, sets up epoll, reads the config (with my friend's ConfigParser class which returns a Config vector), opens listen sockets according to the config, and runs the server loop.

This loop calls an epoll_wait with a 1 second timeout thanks to my EpollInstance class. Epoll has already been populated by all the ListenSocket objects on their construction. On epoll_wait return, I check every returned event in a loop. Since I registered object pointers in epoll data.ptr, I can simply grab the pointer and call my handleEvent() virtual fonction. Because later I will also have other types of objects to register in epoll than just ListenSocket, I opted to make them all derive from a common IOChannel interface (the name is not ideal). The IOChannel class exposes 3 pure virtual functions: a handleEvent(uint32_t events) function, a checkTimers(std::time_t now) function, and a pendingDelete() getter function. 3 classes derive from it: ListenSocket, ClientSession, and CGICall. Each has its own handleEvent() logic, which is the heart of the program.

After the handleEvent loop, I simply run a global checkTimers() on all my IOChannel objects, ensuring that CGI calls or connections never hang indefinitely, and execute all pending deletions (when a ClientSession or CGICall dies, it flags itself for deletion and waits - ensuring that any further related events have a chance to safely abort).

Almost everything else happens in constructors and destructors, for now. When a ListenSocket constructs, it does the interface address resolving, socket opening and binding, listening and adding itself to epoll. ListenSocket is only destructed on server shutdown, and only deletes itself from epoll and closes its fd. When a ClientSession constructs due to a ListenSocket's handleEvent(), it calls accept, adds itself to epoll and lets the server manage its own IOChannel vector and state. On destruction, it removes itself from epoll, 

### CGI Call

Has to be in server's IOChannel vector if we want to check timers on it during the loop.
We could do it differently though: check a CGI Call timer when we check the Client Session timer. The Client Session could take responsibility for the CGI Call.

However, CGI Call has to register itself in epoll. If EPOLLERR or EPOLLHUP arrives and CGI Call has registered its Client Session, we cannot differentiate between CGI Pipes/Client Socket. So CGI needs its own `handleEvent()`, at the very least.

If we don't register CGI Call in server's IOChannel, Client Session has to handle full responsibility: it has to destroy it with itself (this is already the case, code smell), and it has to check its timer along with its own.

If we do register CGI Call, it can check its own timer and it should destroy itself (Client Session should NOT try to destroy it).

It makes more sense for a Client Session to fully own its CGI Call, because that's the way they are logically linked in reality. A Client Session can exist without CGI Call but a CGI Call cannot exist without a Client Session. A CGI Call still needs its own handleEvent and checkTimers though since it has to receive the events from epoll, but its timers would be checked with the Client Session.

On any kind of error, such as EPOLLHUP or a timeout, the CGI Call needs to be deleted and the error needs to go back up to the ClientSession so that it can respond to the client. On EPOLL errors, the CGI will be called by itself, not from Client Session, so it needs a way to notify the Client Session about what happened.

There is a logical paradox in how a CGI Call object should be destroyed if the Client Session owns it, while a CGI Call can be called by itself without a Client Session from EPOLL events. If the event is an error or a last EPOLLIN after which the object should be destroyed, the object has to call the Client Session and ask it to destroy it. But this will be done during the execution of a function of the object, so the destruction will happen before exiting that object's function.

While technically possible, this seems like bad code. Thus, a CGI Call should probably be owned by the server before being owned by its Client Session. The two should simply be linked and exist independently from each other, technically (even though they are logically linked).

If we commit to this idea, we need to decide how a CGI Call acts on an EPOLLERR, last EPOLLIN, or timeout. The first two are from events, they will be called from a handleEvent from server. The third is called from a checkTimers from server also (if CGI is to be independent).

A CGI object should probably hold its own two buffers, one for sending, the other for receiving. The receiving buffer should probably have a limit (we set CGIResponseMaxLength in ServerConfig), but the sending one doesn't need one since we build it ourselves from a capped request.

**Waitpid:** an important detail, once a CGI call is terminated for whatever reason, I need to waitpid it, and since the waitpid needs to be non-blocking, I need to run it in a loop. So I need to keep in memory every pid and set up a wait routine that attempts waitpid on each of them. 