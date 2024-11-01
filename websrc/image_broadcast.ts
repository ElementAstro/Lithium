import { v4 as uuidv4 } from "uuid"; // Import UUID library for generating unique client IDs
import WebSocket, { WebSocketServer } from "ws"; // Import WebSocket library for real-time communication
import dgram from "dgram"; // Import dgram library for UDP communication
import express, { Request, Response, NextFunction } from "express"; // Import Express library for creating HTTP server
import { createServer } from "http"; // Import HTTP library for creating HTTP server
import os from "os"; // Import OS library for network interface information

// Define types for client and message
interface Client {
  username: string;
  isAlive: boolean;
}

interface Message {
  type: string;
  content?: string;
  username?: string;
  to?: string;
  command?: string;
}

// Create Express application
const app = express();

// Enable CORS (Cross-Origin Resource Sharing) for all routes
app.use((req: Request, res: Response, next: NextFunction) => {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Methods", "GET, PUT, POST, DELETE");
  res.header("Access-Control-Allow-Headers", "Content-Type");
  next();
});

// Set static file directory for serving images
app.use("/images", express.static("/dev/shm"));

// Create HTTP server and associate it with Express application
const server = createServer(app);

// Create WebSocket server associated with the HTTP server
const wss = new WebSocketServer({ server });

// Start HTTP server on a specific port
server.listen(8600, () => {
  console.log("HTTP and WebSocket server started on http://localhost:8600");
});

// Global state for connected clients and chat history
const clients: { [key: string]: Client } = {}; // Object to store connected clients
const chatHistory: { user: string; message: string; timestamp: string }[] = []; // Array to store chat history
const MAX_HISTORY = 10; // Maximum number of chat messages to store

// Function to get the broadcast address
const getBroadcastAddress = (): string | null => {
  const interfaces = os.networkInterfaces(); // Get network interfaces

  for (const netInterface of Object.values(interfaces)) {
    for (const net of netInterface!) {
      if (net.family === "IPv4" && !net.internal) {
        // Check for IPv4 and non-internal addresses
        const ipParts = net.address.split(".").map(Number); // Split IP address into parts
        const subnetParts = net.netmask.split(".").map(Number); // Split subnet mask into parts

        // Calculate broadcast address
        const broadcastParts = ipParts.map(
          (part, i) => part | (~subnetParts[i] & 255)
        );
        return broadcastParts.join("."); // Join parts to form broadcast address
      }
    }
  }
  return null; // Return null if no broadcast address found
};

const BROADCAST_PORT = 8080; // Port for broadcasting
const BROADCAST_ADDR = getBroadcastAddress(); // Get broadcast address
const BROADCAST_INTERVAL_MS = 2000; // Interval for broadcasting in milliseconds
const udpSocket = dgram.createSocket("udp4"); // Create UDP socket

udpSocket.on("listening", () => {
  udpSocket.setBroadcast(true); // Enable broadcast on UDP socket
  console.log(
    `UDP socket is listening and ready to broadcast on port ${BROADCAST_PORT}`
  );
});

// Broadcast message at regular intervals
setInterval(() => {
  const message = Buffer.from("Stellarium Shared Memory Service"); // Message to broadcast
  if (BROADCAST_ADDR) {
    udpSocket.send(
      message,
      0,
      message.length,
      BROADCAST_PORT,
      BROADCAST_ADDR,
      (err) => {
        if (err) {
          console.error(`Error sending broadcast message: ${err}`);
        } else {
          console.log(
            `Broadcast message sent to ${BROADCAST_ADDR}:${BROADCAST_PORT}`
          );
        }
      }
    );
  } else {
    console.error("No broadcast address found.");
  }
}, BROADCAST_INTERVAL_MS);

udpSocket.bind(BROADCAST_PORT); // Bind UDP socket to port

const noop = () => {}; // No-operation function
const heartbeat = function (this: WebSocket) {
  this.isAlive = true; // Set client as alive
};

// Handle new WebSocket connections
wss.on("connection", (ws: WebSocket & { id?: string }) => {
  const clientId = uuidv4(); // Generate unique client ID
  ws.id = clientId; // Assign ID to WebSocket connection
  clients[clientId] = { username: `User${clientId.substring(0, 6)}`, isAlive: true }; // Default username

  console.log(`Client ${clientId} connected`);
  ws.isAlive = true; // Set client as alive
  ws.on("pong", heartbeat); // Listen for pong messages to confirm client is alive

  // Send chat history to the new client
  ws.send(JSON.stringify({ type: "CHAT_HISTORY", messages: chatHistory }));

  // Notify all clients of the new connection
  const newClientMessage = {
    type: "SERVER_MSG",
    message: `${clients[clientId].username} connected`,
  };
  wss.clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify(newClientMessage));
    }
  });

  // Handle incoming messages from clients
  ws.on("message", (data: WebSocket.Data) => {
    const message: Message = JSON.parse(data.toString()); // Parse incoming message
    const timestamp = new Date().toLocaleString(); // Get current timestamp

    if (message.type === "MESSAGE") {
      // Save to chat history
      chatHistory.push({
        user: clients[clientId].username,
        message: message.content!,
        timestamp,
      });
      if (chatHistory.length > MAX_HISTORY) chatHistory.shift(); // Limit history

      // Broadcast the new message
      wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
          client.send(
            JSON.stringify({
              type: "MESSAGE",
              user: clients[clientId].username,
              content: message.content,
              timestamp,
            })
          );
        }
      });
    } else if (message.type === "SET_USERNAME") {
      // Set username for the client
      clients[clientId].username = message.username!;
      ws.send(
        JSON.stringify({
          type: "SERVER_MSG",
          message: `Username set to ${message.username}`,
        })
      );
    } else if (message.type === "PRIVATE_MESSAGE") {
      // Handle private messages
      const recipient = Object.keys(clients).find(
        (id) => clients[id].username === message.to
      );
      if (recipient) {
        wss.clients.forEach((client) => {
          if ((client as WebSocket & { id?: string }).id === recipient && client.readyState === WebSocket.OPEN) {
            client.send(
              JSON.stringify({
                type: "PRIVATE_MESSAGE",
                from: clients[clientId].username,
                content: message.content,
              })
            );
          }
        });
      }
    } else if (message.type === "TYPING") {
      // Notify others that the user is typing
      wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN && (client as WebSocket & { id?: string }).id !== ws.id) {
          client.send(
            JSON.stringify({ type: "TYPING", user: clients[clientId].username })
          );
        }
      });
    } else if (message.type === "FILE") {
      // Optionally handle file data here (this would require encoding)
      console.log(`File received from ${clients[clientId].username}:`, message);
    } else if (message.type === "COMMAND") {
      // Handle commands
      if (message.command === "/users") {
        const userNames = Object.values(clients)
          .map((client) => client.username)
          .join(", ");
        ws.send(
          JSON.stringify({
            type: "SERVER_MSG",
            message: `Connected users: ${userNames}`,
          })
        );
      } else if (message.command === "/help") {
        const commandsList =
          "/users: List all connected users\n/help: Show this help message";
        ws.send(JSON.stringify({ type: "SERVER_MSG", message: commandsList }));
      }
    }
  });

  // Handle client disconnection
  ws.on("close", () => {
    console.log(`Client ${clientId} disconnected`);
    const username = clients[clientId].username;

    delete clients[clientId]; // Remove client from list

    const messageObj = {
      type: "SERVER_MSG",
      message: `${username} disconnected`,
    };

    // Notify all clients of the disconnection
    wss.clients.forEach((client) => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(JSON.stringify(messageObj));
      }
    });
  });

  ws.on("error", console.error); // Handle WebSocket errors
});

// Periodic health check for clients
const interval = setInterval(() => {
  wss.clients.forEach((ws) => {
    if ((ws as WebSocket & { isAlive?: boolean }).isAlive === false) {
      console.log(`Client ${(ws as WebSocket & { id?: string }).id} did not respond to a ping, terminating.`);
      return ws.terminate(); // Terminate unresponsive clients
    }
    (ws as WebSocket & { isAlive?: boolean }).isAlive = false;
    ws.ping(noop); // Send ping to clients
  });
}, 3000);

wss.on("close", () => {
  clearInterval(interval); // Clear interval on server close
});

// Handle process exit
process.on("exit", () => {
  wss.close(() => {
    console.log("WebSocket server closed");
  });

  udpSocket.close(() => {
    console.log("UDP socket closed");
  });
});

console.log("WebSocket server started on ws://localhost:8600");
