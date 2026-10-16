# Webserv, a 42 common core project

[Subject](https://cdn.intra.42.fr/pdf/pdf/222787/en.subject.pdf)

### Introduction

webserv is a very basic web server that only handles GET, POST, DELETE. Otherwise, it acts like a very simple web server that should be compatible with any web client, like a browser.

The project is in the process of being built. This README is a collection of thoughts and ideas pertaining to development, but will be cleaned up once the project is complete.

### Development thoughts

#### What is a web server

A web server is a program that should run on a machine, be reachable on the internet, and serve clients. The way to serve clients is to respond to HTTP requests to allow access to the websites hosted on the server. A single server can host multiple websites.

A website is a collection of pages and resources such as index.html or cat.jpg. The pages should be hypertext documents. They are stored somewhere the server can reach easily (probably locally). If the hypertext documents ask for any additional resources (images, scripts...) the browser will send more requests for those. The server should fetch them and respond.

A web server can also handle cookies, post requests, users signing in, logging in, etc...

The kernel handles TCP/IP communication, so the only thing we _should_ have to handle is the HTTP requests.

An HTTP request looks like:

GET / HTTP/1.1
Host: localhost:8080
User-Agent: curl/8.18.0
Accept: */*

If the browser were looking for index.html, for example, we would see

GET /index.html HTTP/1.1

The other lines (with colons) are headers, they provide context about the request or response. Some, like Content-Length, are necessary for proper communication.

An HTTP response looks like:

HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 13
Hello, world!

#### Current progression

In my very basic webserv.cpp test, I am opening a socket and listening in on a port. I then wait (forever) for a connection, read it (completely), and reply directly with a generic message, then leave.

In the actual webserv project, we'll need to:

1. Not wait forever for a connection
2. Handle incomplete connections (not hang up on reading)
3. Handle multiple connections in parallel
4. Reply with whatever we want the website to have

To avoid waiting forever for a connection, we'll need to use poll() (or equivalents like select()).

To handle incomplete connections, we'll need to read in a non-blocking manner and store requests in memory.

To handle multiple connections in parallel, we'll need to store requests in memory and link them to their origin. We'll have to work in parallel. We should probably implement a reasonable connection drop timer so as to not hold a connection in memory for hours.

We should eventually host multiple websites, so we might have to open multiple sockets for multiple ports. As for the files, this will have to be inspecting the request, seeing if the file exists (and if not, using the configuration file for a default page to display, e.g 404 page or index page), and serving the file in the response (in the body, after the header). For POST and DELETE requests, if the configuration file allows for them, we'll have to create a new file with the contents of the POST request, or delete a file in the DELETE request.

The first step should be researching poll(), HTTP/1.1 (and other versions) using MDN or the RFCs, how to use non-blocking operations, and how to serve connections in parallel.

#### Questions to investigate

- Do we need to open a second listen socket on a different port to host several websites? Or can we use the Host header?
- How should the server stop? Ctrl+C? Should we catch the signal to properly free memory? Should anything else happen?
- What if we receive a method that we shouldn't handle as per subject guidelines (OPTIONS, HEAD, ...)? Do we ignore the request or what do we send back? Do we print a warning at least, maybe? Idea: send back 402 Payment Required (pay me to code this, otherwise I won't) Serious answer: 405 Method Not Allowed - Other, non-serious answer: 418 I'm a teapot - Better serious answer: 501 Not Implemented
- If method is not known or anything else is malformed, send back 400 Bad Request
- What status code and status message should we send back? (see MDN HTTP documentation)
- What headers should we handle, if any?
- Should we maybe cache some files, instead of reading from disk permanently to serve the exact same file to 100s of clients? (probable answer: ideally yes, but Payment Required)
- What if it's been 30 minutes that a connection has been open and we haven't received a complete request? Timeout? What timeout value? 408 Request Timeout?
- 413 Content Too Large response? Should we define a max length for content bodies in the config file? (Answer: yes, read the subject)
- Implement rate limiting? 429 Too Many Requests? (probable answer: why not for fun, otherwise Payment Required)
- Use 505 HTTP Version Not Supported for any non-supported version? (like, any other than the one we're using, really?)

Sub-questions about status codes: ([documentation](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status))

- Some requests can have the "Expect: 100-continue" header - should we handle that? My guess: no.

Request methods:

- We have to handle GET, POST and DELETE
- GET asks for a resource (file), it should be responded to with either 200 OK and the resource, or an error (e.g 404 Not Found)
- POST sends data to the server - it is up to the server to know what to do with it. It could be a form for creating a user account, for example. If a resource is created server-side, 201 Created should be sent.
- DELETE requests for a resource to be deleted. The server can send back 200 OK, or 204 No Content (meaning success, but nothing more to say), or 202 Accepted meaning the deletion has not happened yet but the request has been processed.
- HEAD is a simple method, it is a GET that expects a response without a body (a GET with only HEADers)
- A 404 Not Found response can include a body of the 404 html page. Per subject guidelines, we should have a default 404 page and be able to define one in the configuration files.

Method questions:

- Examples show responses with Date: etc... Do we have to do this? Send back the metadata of the file beyond just Content-Length?
- What the hell should we do when we receive POST? Just allow writing whatever file into the server? What if this replaces a file?...

#### Thinking about program structure

The server will handle many concurrent connections and requests.

First, it's a good idea to define classes for frequently used objects. For example, a Socket class for sockets could be useful: open the socket at construction, close it at destruction.

Second, we're going to need to use some kind of data array to store information about concurrent connections (sockets) and their active request. It is possible that an active connection has sent a request, we have sent the response back, and nothing's happening for some time - the connection should remain active instead of being dropped (establishing a TCP connection is expensive, so we should keep it open for any potential additional requests). Then, we need to establish a way to receive a request, possibly in many chunks, and we should store data about the request as we receive it. A request has a Method (GET), Resource (/index.html), Protocol (HTTP/1.1), a number of Headers (Content-Type: text/html), and a Body (a bunch of data which length is specified with the Content-Length header).

We should probably have a "Request" class, with attributes to store Method, Protocol, etc...

Maybe we should have a "Connection" class which holds a Request object, as well as a Socket object?

### Resources

[MDN HTTP Overview](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Overview) and further documentation pages

[Computerphile web server in 25 lines (in Rust)](https://youtu.be/7GBlCinu9yg) and other Computerphile vids
