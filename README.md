#ProxyServerForPostgresql

Usage: ./proxy "proxy server ip" "proxy server port" "database host ip" "database port" "file for logs"

Before starting, please disable the SSL.

How does it works? 

Clients can connect to the proxy server on a specific port and send requests. The proxy server redirects requests received from the client to the SQL server. During the redirection process, the program parses the information contained in the request and logs it to a file. Upon receipt of responses from the SQL server, the response is sent back to the client.

Interaction is shown in the picture:

![image](https://user-images.githubusercontent.com/68387788/117286193-8a507c00-ae71-11eb-919c-33fa95088be7.png)


Some information from Postgresql documentation:

The protocol has separate phases for startup and normal operation. In the startup phase, the frontend opens a connection to the server and authenticates itself to the satisfaction of the server. (This might involve a single message, or multiple messages depending on the authentication method being used.) If all goes well, the server then sends status information to the frontend, and finally enters normal operation. Except for the initial startup-request message, this part of the protocol is driven by the server.

During normal operation, the frontend sends queries and other commands to the backend, and the backend sends back query results and other responses. There are a few cases wherein the backend will send unsolicited messages, but for the most part this portion of a session is driven by frontend requests.

Termination of the session is normally by frontend choice, but can be forced by the backend in certain cases. In any case, when the backend closes the connection, it will roll back any open (incomplete) transaction before exiting.

An example of log:

![image](https://user-images.githubusercontent.com/68387788/117294118-f08dcc80-ae7a-11eb-9248-6a95f4c9008c.png)



