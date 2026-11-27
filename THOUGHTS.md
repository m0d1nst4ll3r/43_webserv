Request parsing thoughts:

#### Why ReceiveStatus and ReceiveResult?

When calling recv, we have had an EPOLLIN just before. The recv call can either:
a. Succeed with >= 1 return (read at least one byte)
b. Fail with -1 return
c. "Succeed" with 0 return

When it succeeds, we read the bytes and jump to next state if needed.

When it fails with -1, we want to just close the connection, because some unexpected error happened on the socket. We don't even build a response, because the socket is compromised, just close and that's it.

When it "succeeds" with 0, it just means there was nothing to read. Why? We just received EPOLLIN, clearly there WAS something to read. This is also an unexpected error, so we will also just close the socket and that's it.

The socket recv() function should just notify of an error (-1 or 0), it doesn't need to discriminate.

#### An additional note on recv() return

A -1 return is an error. We cannot check errno, so we cannot know what type, if recoverable or not. So we should still close no matter what.

A 0 return, however, is NOT an error. It is simply an indication that the client has closed its write-end. It is an indication that keeping the connection alive after sending all the data is useless, because the client will never send anything back.

It is therefore interesting for recv() to notify the caller that 0 was received. The simplest way to do this is to return an empty string.

There should still be no need to clutter the code with a special structure anyway.

#### char* instead of string

We could be using char* instead of string where null bytes are possible. I started out by using string for simplicity, but we will have to read null bytes, because we will have to read binary.

I don't truly know whether it is best practice to use string or char* for data containing null bytes. Technically, string can handle it perfectly. E.g `std::string data("AB\0C", 4)`.

- Null bytes (continuation of char*)

Where are null bytes used anyway?

Null bytes are found only in the body of HTTP messages, apparently.

We can only be using char*, as we are right now, for fixed length buffers for recv().

#### HTTPRequest class and construction

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

#### Request Parsing Details

General syntax checks

- Header Line: `name:(OWS)value(OWS)\r\n` means no whitespace before or after header name, but whitespaces (tabs, spaces) possible before and after value
- Header Name: a-z A-Z 0-9 and !#$%&'*+-.^_`|~ however header names are transformed into lowercase, also cannot be empty
- Header Value: can be empty, and can contain any byte except 7F (del) and anything from 00 to 1F (before space), except tab (09), this means from 80 to ff is allowed, and from 20 to 7E is allowed, and 09 is allowed. (OWS) is trimmed at edges of value.
- Method: like header name, cannot be empty, yes this means get_03 is valid, this is case-sensitive meaning get and GET are not the same
- Protocol: HTTP/X.X exactly, X is replaced by one digit
- Target: a-z A-Z 0-9 `-._~` non-reserved characters and `:/?#[]@!$&'()*+,;=` reserved characters, these are all allowed, though any character outside of that is allowed by encoding it with %hh

Note that for target (the most important), a % has to be followed by 2 digits. Literal spaces are not allowed, non ascii characters should be percent encoded (like é)

Specific header value semantics

- content-length obviously must only contain digits
- connection has specific values like "close" or "keep-alive"

#### Target parsing

`/path/to/resource?name=hello%20world`

Whatever follows `?` is a query. In this case the server needs to separate `/path/to/resource` from `?name=hello%20world`. The former is the file path, the latter is a query. The server should not look for a file named `resource?name=hello%20world`.

In the case that the resource is a static file, the server _ignores_ the query. However, in the case that the resource is _dynamic_ (handled by application code or a script), we pass the raw query string to it (here, `name=hello%20world`). The application should decode the string itself.

In our case, the query is only useful in a CGI script. But on regular servers, they can be useful for much more (don't ask me what).

`/path/to/myfile.html?key1=value1&key2=value2#SomewhereInTheDocument`

Whatever follows `#` is a fragment. This is used by the client (browser, probably) to set an anchor in the resulting page. This works with markdown files, for example.

The server will never receive this. Clients do not send fragments as part of the request, they just use it for themselves. `#` is not a valid unencoded URI character.

#### Legal Protocol, Legal Method

For now I set it up so that we can, directly at parse, say that `ASDF` is not a recognized method anywhere and so we should not even return a 501 error, but rather 400. This is overkill for our scope, not to mention that technically `ASDF` *could* be a valid method somewhere because the HTTP specification does not enforce any method, but rather allows methods to be extended per user need. The way some web servers do it is that they have a list of hardcoded methods that are valid. However, some others do not. Honestly, for our case, the simplest is to just say we do not handle `ASDF` rather than say `ASDF` is not a valid method. This eliminates the need for "legal method" in ServerConfig.

Similarly, HTTP/9.9 is not a protocol that exists anywhere, but the HTTP spec says HTTP/X.X is a valid protocol format. You might be confused about HTTP/2 and HTTP/3 but these protocols don't even use the same protocol framing at all, so don't even bother thinking about them. For our basic usecase, HTTP/X.X is the framing, and HTTP/1.2 might exist, but we just don't handle it. At least, I think this makes much more sense. This also eliminates the need for "legal protocol" in ServerConfig.

My recommendation is that we just don't send anything to the Request constructor other than the string.

#### \r\n vs \n

On this question, modern web servers like nginx and apache accept \n. They use tolerant parsing. However, this can make the server vulnerable to request smuggling attacks if the request goes through several parsers that might be differently configured. Security-conscious engines do not tolerate bare \n and may even reject it outright instead of ignoring it.

For our case, either is fine. Tolerating \n is convenient for our own tests.

#### ALL of the ERRORS

In order that we might catch the errors.

- No code: If the socket has any issue and we can't respond
- 429: Too many requests (we will need to implement this!)
- 400: Malformed input, method is invalid, or too long, or protocol is invalid, or too long, or received bytes are too much (before body)
- 414: Target field is too long
- 501: Server does not handle this method (might want to bundle invalid methods in there like `ASDF`)
- 505: Server does not handle this protocol (protocol should be parsed like HTTP/X.X)
- 431: Header line is too long (a single line)

- 405: Website config does not allow this method on target resource
- 404: Can't find resource


Headers present in errors:
- `Connection: close` included when we stopped reading the TCP stream mid-header to preserve memory. Connection sync is lost so we cannot resume.
- `Content-type` with something like `text/html` if an error page is included, with its `Content-length`.
- `Date` and `Server` metadata headers to identify who rejected the request (because it might not be our server, it might be something in-between).
- `Retry-After` with how many seconds to wait. Useful for some errors like 429 (rate limiting).
- `Allow` followed with allowed methods like `GET, POST`, this is used for 405 errors.

#### HTTP protocols?

HTTP/0.9 HTTP/1.0 HTTP/1.1

The subject suggests 1.0 only.

However, the subject also mentions chunked encoding, which is a 1.1 requirement. This is probably an error: the subject previously required 1.1, but was changed to 1.0. They probably forgot to remove the line about chunked encoding, or decided to leave it for whoever decides to go for 1.1.

0.9 should be ignored: it is obsolete and only required for servers that want to be backwards compatible for some reason. We don't, and handling it complicates things.

We could simply go for 1.0 as suggested by the subject. Connections are always closed after response in 1.0 and there is no chunked encoding.

We can also go for 1.1. This is what I blindly went for before even deciding anything, because it just made sense, but if only 1.0 is needed, then we can backtrack and redesign.

1.1 adds chunked encoding, enforces the host header for virtual hosting (which we might want, or might not want to implement). For now we don't implemented virtual hosting. 1.1 also adds a "expect: 100-continue" header if the client wants to send the body separately, which complicates things further. If we don't implement it we should at least handle it gracefully. It also adds different request-target formats like `http://example.com/path` instead of `/example.com/path` - we can totally decide to reject that format.

Note: `Connection: keep-alive` is an opt-in for HTTP/1.0, we should have a special case for it to override the default 1.0 close behaviour. Or we can ignore it. This is an unofficial extension.
Note: HTTP/1.2 is handled by *some* servers by using the highest version of HTTP 1 that they support, so 1.1 - this is recommended by RFC 9112, but many modern servers will choose to strictly reject the request.

#### POST returns

- `201 Created`: new file was created
- `204 No Content`: file was modified
- `409 Conflict`: parent directory does not exist, or the file exists and is a directory
- `403 Forbidden`: no perms
- `500 Internal Server Error`: any other I/O error

Actually when file is modified, we could return `200 OK` instead. The reasoning behind the `204 No Content` response is that this is what actual websites do when a `POST` operation changed an already-existing file. This is apparently used for the browser to know that it does not need to load any new resource. But honestly, I'm not 100% on this.

#### DELETE returns

- `204 No Content`: file was deleted
- `404 Not Found`: file doesn't exist, or parent directory does not exist
- `403 Forbidden`: no perms, or file is a folder
- `500 Internal Server Error`: any other I/O error

## Program Logic Flow

### Concept

Listen Socket is readable -> Client Socket opens (Client Session is created)

At start, Client Session listens for a request

It wants \r\n\r\n an handles errors which might cause a response

Once it's received, it parses and creates a Request, which might again cause an error response

It decides whether it needs to listen for a body, if so, it receives the body either with length or in chunked (might cause error response)

Once everything is received, we are processing the request and routing it. Is it asking for a static resource or a CGI resource ?

Static resource -> seek and serve (depending on what we found)

CGI resource -> set up CGI (pipe, fork, dup, envp, execve) -> write to CGI -> read from CGI -> respond

For either choice, we end by building a response

After all this, depending on the connection state (close or keep-alive), we either close or continue listening for another request

### Visual

(see excalidraw file)

Session starts
     |
     |---------------------------------┐
     V            error?               |
Read request   --------------┐         |
     |                       |         |
     |                       |         |
     V            error?     |         |
Read body      --------------|         |
     |                       |         |
     |                       |         |
     V            error?     |         |
Process request -------------|         |
 |       |                   |         |
 | cgi   | no cgi            |         |
 V       |        error?     |         |
CGI Call |      -------------|         |
 |       |                   |         |
 |       |                   |         |
 V       V        can cause  |         |
Build response  <------------|         |
(resp may be)     response?  |         |
   (error)                   |         |
    close?                   |         |
      |         no           |         |
      |--------------------------------┘
  yes |                      |
      V                      |
Close session  <-------------┘

### Error Logic

Errors come in two forms: one tries to respond (if possible), the other just closes the socket.

If the error cannot respond, e.g the socket unexpectedly died on us, we should just close. However, as long as the connection is still active, we should respond.

Because errors can cause responses, they are similar to successful responses. All that changes is the content of the response, but the same logic should be reused.

Errors can also come at different stages of the logic flow. For example, they can come right after accepting the session (rate limiting error, 429). They can happen at almost every stage.

Since errors and exceptions go hand in hand, I really want to use exceptions.

### To add

- Structure matching error codes to error descriptions as per HTTP specs
- Structure to register IP addresses with amount of recently sent requests, to do rate limiting
- Logic at client session creation to check for rate limit and automatic 429 response with correct try-after header
- Logic on request processing to lookup target config, e.g `/contact/toto/file` will first look for a config for target `/contact/toto/file` then `/contact/toto` then `/contact` then `/`

## Thoughts

### Response building

A Response has:

- `protocol code-status` (code-status go together and should be defined in a struct or enum or something)
- Headers
- Body

We have a Response object. How do we build it? The way I envisioned it was directly passing the Request to the Response constructor and letting it do its thing.

This can work, however this implies all of the request-processing logic will be found in the Response constructor. Is this way acceptable? Is there a better way to do things?

A: Not a good way to do this, because the request-processing logic has to include potentially doing a CGI Call. It makes no sense for the *Response* constructor to take a request and possibly end by having none of its fields filled (protocol, status code,etc).

Note: a Response made from CGI is very specific. The CGI will return a string of data, which needs to be parsed to determine how to build the Response. An error can come from this parsing. The point is, it is completely different to how we build a Response from other stuff.

Actually, let's just document ALL of the ways Responses can be built, right now, to start having a better picture.

#### Replying with file

Any GET request on a _static_ resource that can be read needs to be built from the actual file contents.

It has to read the file first, in a loop, store that in a buffer, and send that as a response.

A very basic way is storing in a buffer, copying that into the Response object, then building a ready-made full string to send back through TCP. For big files, this is inefficient, but our web server is basic and simple.

#### Replying from CGI

Any request on a _dynamic_ resource results in a CGI Call which returns data as a string.

That data needs to be parsed to determine whether its syntax is right, and to fill a Response object which can then build the ready-made full string to send back.

#### Replying after POST/DELETE

The creation/deletion operation should be executed and, depending on its output, we might return a bunch of different codes.

There should be no headers for this, and no body. Just a simple code.

#### Error reply

This can have a body, depending on the error and depending on the config (e.g 404 responses with website-specific document).

This can also have headers, like `retry-after` or `allow`. Each error case is different.

#### Conclusion

The request processing should probably first decide whether the resource is static or dynamic, and then if it is static, what the request is (get, post, delete). This yields 4 possible outcomes.

Each of the 3 static paths can end in a Response and a state switch, whereas the dynamic path can switch to a WAITING_CGI or some state, with the Response coming in later when the CGI does (or does not) yield a result.

Since any errors in the static paths result in a response anyway, it's possible for them to directly generate error responses depending on what happens during the operation. This might be the best way to handle this. The functions are defined to return a Response, and any error path would just early-return with a specially built error response.

Any CGI error could simply work the same as a success: notify ClientSession to change its state, register epoll, and give it a Response.

This only accounts for errors during Request Processing and CGI Call, but does not account for earlier errors like rate-limiting, and request reading. In this case, we might be able to use exceptions in a way that makes sense.

## Prompts

### TMP1

I am thinking and planning around my web server program. I would like some guidance on my choices because I have just arrived to Object Oriented Programming and do not trust my instincts yet.

I have a `ClientSession` class that holds `Request*` and `Response*` objects. This felt intuitively right to me: a Session might not always hold a Request or Response (like before receiving any data, or after sending back a response with keep-alive). I also designed my Request class to construct with a string: specifically, all of the data that I receive before the body (up to the first `\r\n\r\n`). It parses the string and, if all of the request syntax is correct, it constructs with its `_protocol`, `_method`, and other fields holding the right data. Note that this constructor only verifies HTTP syntax, it doesn't process anything yet. I then process whether I should read a body next, and add the body to the Request object. Then, I need to process the Request. My instinct was to create my Response class so that I can send a Request to it

### TMP2

I'm thinking and planning around my web server. Specifically, thinking about how error responses fit in my logic. My logic flow for a Client Session will look something like this:

1. Session Start - EPOLLIN came in on a Listen Socket
2. Read Request - until \r\n\r\n
3. Read Body - if there is one
4. Process Request - determine how to reach/handle the targeted resource, might cause a CGI Call
5. CGI Call - execute script
6. Build Response and Send - from step 4 or 5

Errors can happen at most stages. Some examples:

1. 429 - rate limiting, detected right after accepting on the Listen Socket
2. 413 - too big content-length
3. 408 - timeout, data came in too slowly
4. 405 - method not allowed by website config
5. 502 - CGI response malformed (rare, but might happen)
6. At this point, there should be no other error than a send timeout which causes no response

These example errors all cause the building and sending of a response. I haven't built a function for Response building yet, but am thinking about how to implement this. Here are my thoughts:

I have a `ClientSession` class containing a `Request*` and `Response*`. I start building my Request right at step 2 (after finding \r\n\r\n) and complete it at step 3 (with the body). If everything goes right, I will build my Response at step 6, but I might build it earlier. 