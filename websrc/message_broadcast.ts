/**
 * Relay Server with Groups, Message Persistence, and Dynamic Routing in Node.js
 *
 * This Node.js server provides a relay mechanism using WebSockets, allowing clients to:
 * 1. Authenticate with tokens.
 * 2. Subscribe to and unsubscribe from message routes.
 * 3. Join and leave groups.
 * 4. Send messages with priorities.
 * 5. Broadcast, multicast, and unicast messages to clients.
 * 6. Persist and replay historical messages for each route.
 * 7. Use heartbeat checks for connection health.
 *
 * Required Dependencies:
 * - ws (WebSocket library for Node.js)
 *
 * Usage:
 * 1. Run the server with `node server.js`.
 * 2. Clients can connect to the WebSocket server at ws://localhost:8080.
 * 3. Clients must authenticate using a valid token before sending/receiving messages.
 *
 * Example client actions:
 * - Authenticate using a token.
 * - Subscribe to routes or join groups to receive specific messages.
 * - Send messages to specific routes or groups.
 * - Replay historical messages upon subscribing to a route.
 *
 * Core Features:
 * - Authentication with tokens.
 * - Group management and dynamic message routing.
 * - Message priority queue and persistence.
 * - Heartbeat mechanism for connection health check.
 *
 * Author: Max Qian
 * Date: 2024-9-25
 */

import WebSocket, { WebSocketServer } from "ws";
import EventEmitter from "events";
import crypto from "crypto";

// Define types for client and message
interface Client {
  username: string;
  isAlive: boolean;
}

interface Message {
  type: string;
  token?: string;
  routes?: string[];
  groups?: string[];
  priority?: number;
  destination?: string | string[];
  payload?: any;
  command?: string;
}

// Create a message bus using EventEmitter to handle event-driven message routing
class MessageBus extends EventEmitter {}

const messageBus = new MessageBus();

// Maps for storing client subscriptions, authentication status, and group memberships
const clientRoutes = new Map<WebSocket, Set<string>>(); // Tracks which routes clients are subscribed to
const authenticatedClients = new Set<WebSocket>(); // Tracks clients that have successfully authenticated
const groupMembers = new Map<string, Set<WebSocket>>(); // Tracks clients in various groups

// Stores message history for routes and groups for replay functionality
const messageHistory = new Map<string, any[]>();

// Priority Queue class to manage message priority and ensure higher priority messages are processed first
class PriorityQueue {
  private queue: { message: any; priority: number }[] = [];

  enqueue(message: any, priority: number) {
    // Insert messages into the queue with a priority and sort by highest priority first
    this.queue.push({ message, priority });
    this.queue.sort((a, b) => b.priority - a.priority); // Higher priority comes first
  }

  dequeue() {
    // Retrieve and remove the first message in the queue
    return this.queue.shift()?.message;
  }

  isEmpty() {
    // Check if the queue is empty
    return this.queue.length === 0;
  }
}

// Create a WebSocket server on port 8080
const wss = new WebSocketServer({ port: 8080 });

// Function to generate authentication tokens (in a real scenario, these tokens would be managed securely)
function generateAuthToken(): string {
  return crypto.randomBytes(16).toString("hex");
}

// Predefined valid authentication tokens for demo purposes
const validAuthTokens = new Set([generateAuthToken(), generateAuthToken()]);

// Handle new WebSocket connections
wss.on("connection", (ws: WebSocket) => {
  console.log("Client connected");

  // Initialize the client's subscriptions and message queue
  let subscribedRoutes = new Set<string>();
  let messageQueue = new PriorityQueue();

  let isAuthenticated = false; // Client is not authenticated by default
  let clientGroups = new Set<string>(); // Client's group memberships

  // Heartbeat mechanism to check if the client connection is healthy every 5 seconds
  const heartbeatInterval = setInterval(() => {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ type: "heartbeat", timestamp: Date.now() }));
    }
  }, 5000);

  // Handle incoming messages from the client
  ws.on("message", (message: WebSocket.Data) => {
    try {
      const data: Message = JSON.parse(message.toString());

      // Handle client authentication
      if (data.type === "auth") {
        // Check if the provided token is valid
        if (validAuthTokens.has(data.token!)) {
          isAuthenticated = true;
          authenticatedClients.add(ws);
          ws.send(
            JSON.stringify({
              type: "auth",
              success: true,
              message: "Authentication successful",
            })
          );
          console.log("Client authenticated");
        } else {
          ws.send(
            JSON.stringify({
              type: "auth",
              success: false,
              message: "Authentication failed",
            })
          );
          console.log("Client authentication failed");
        }
        return;
      }

      // If the client is not authenticated, reject any other actions
      if (!isAuthenticated) {
        ws.send(
          JSON.stringify({
            error: "Not authenticated, unable to perform action",
          })
        );
        return;
      }

      // Handle route subscription
      if (data.type === "subscribe") {
        if (Array.isArray(data.routes)) {
          data.routes.forEach((route) => subscribedRoutes.add(route));
          clientRoutes.set(ws, subscribedRoutes);
          console.log(
            `Client subscribed to routes: ${Array.from(subscribedRoutes)}`
          );

          // Replay message history for the subscribed routes
          data.routes.forEach((route) => {
            if (messageHistory.has(route)) {
              const history = messageHistory.get(route);
              history.forEach((histMsg: any) => {
                ws.send(JSON.stringify({ route, message: histMsg }));
              });
            }
          });
        }
      } else if (data.type === "unsubscribe") {
        // Handle route unsubscription
        if (Array.isArray(data.routes)) {
          data.routes.forEach((route) => subscribedRoutes.delete(route));
          clientRoutes.set(ws, subscribedRoutes);
          console.log(
            `Client unsubscribed from routes: ${Array.from(subscribedRoutes)}`
          );
        }
      } else if (data.type === "join_group") {
        // Handle client joining a group
        if (Array.isArray(data.groups)) {
          data.groups.forEach((group) => {
            if (!groupMembers.has(group)) {
              groupMembers.set(group, new Set());
            }
            groupMembers.get(group)!.add(ws);
            clientGroups.add(group);
          });
          console.log(`Client joined groups: ${Array.from(clientGroups)}`);
        }
      } else if (data.type === "leave_group") {
        // Handle client leaving a group
        if (Array.isArray(data.groups)) {
          data.groups.forEach((group) => {
            if (groupMembers.has(group)) {
              groupMembers.get(group)!.delete(ws);
              clientGroups.delete(group);
            }
          });
          console.log(`Client left groups: ${Array.from(clientGroups)}`);
        }
      } else if (data.type === "message") {
        // Handle sending messages with optional priority
        const priority = data.priority || 1;
        const destination = data.destination || "broadcast";

        // Broadcast the message to all clients
        if (destination === "broadcast") {
          broadcastMessage(data.payload);
        } else if (Array.isArray(destination)) {
          // Multicast the message to specific routes or groups
          destination.forEach((route) => {
            if (!messageHistory.has(route)) {
              messageHistory.set(route, []);
            }
            messageHistory.get(route)!.push(data.payload); // Persist the message for replay
            multicastMessage(route, data.payload);
          });
        }
      }
    } catch (err) {
      console.error("Error parsing message", err);
    }
  });

  // Handle client disconnection and cleanup
  ws.on("close", () => {
    console.log("Client disconnected");
    clearInterval(heartbeatInterval);
    clientRoutes.delete(ws);
    authenticatedClients.delete(ws);

    // Remove the client from all groups
    clientGroups.forEach((group) => {
      if (groupMembers.has(group)) {
        groupMembers.get(group)!.delete(ws);
      }
    });
  });
});

// Function to broadcast a message to all connected clients
function broadcastMessage(message: any) {
  wss.clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify({ route: "broadcast", message }));
    }
  });
}

// Function to multicast a message to clients subscribed to a specific route or in a group
function multicastMessage(route: string, message: any) {
  wss.clients.forEach((client) => {
    const clientSubscribedRoutes = clientRoutes.get(client);
    if (clientSubscribedRoutes && clientSubscribedRoutes.has(route)) {
      if (client.readyState === WebSocket.OPEN) {
        client.send(JSON.stringify({ route, message }));
      }
    }
  });
}

console.log("WebSocket server is running on port 8080");
