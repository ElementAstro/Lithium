import asyncio
import json
import subprocess
import logging
from typing import Dict, Any

# Set up logging
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s')


class JSONTCPServer:
    """
    An asynchronous TCP server that handles JSON-formatted commands and sends back JSON-formatted responses.
    """

    def __init__(self, host='127.0.0.1', port=8888, max_clients=10):
        """
        Initialize the JSONTCPServer with the server's host, port, and maximum number of clients.

        Args:
            host (str): The server's hostname or IP address.
            port (int): The server's port number.
            max_clients (int): The maximum number of concurrent clients.
        """
        self.host = host
        self.port = port
        self.max_clients = max_clients
        self.active_clients = 0
        self.semaphore = asyncio.Semaphore(
            max_clients)  # Limit concurrent connections
        self.server = None
        self.clients = []

    async def handle_client(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        """
        Handle incoming client connections and process their requests.

        Args:
            reader (asyncio.StreamReader): The stream reader for the client connection.
            writer (asyncio.StreamWriter): The stream writer for the client connection.
        """
        async with self.semaphore:
            self.active_clients += 1
            client_info = writer.get_extra_info('peername')
            self.clients.append(writer)
            self.log_client_activity(client_info, "connected")

            try:
                while True:
                    data = await reader.readline()
                    if not data:
                        break
                    message = data.decode().strip()
                    logging.info(f"Received {message} from {client_info}")
                    response = await self.process_command(json.loads(message))
                    writer.write((json.dumps(response) + '\n').encode())
                    await writer.drain()
            except Exception as e:
                logging.error(f"Error handling client {client_info}: {e}")
            finally:
                self.disconnect_client(writer)
                self.log_client_activity(client_info, "disconnected")

    async def process_command(self, command: Dict[str, Any]) -> Dict[str, Any]:
        """
        Process a JSON command and return a JSON response.

        Args:
            command (Dict[str, Any]): The JSON command received from the client.

        Returns:
            Dict[str, Any]: The JSON response to send back to the client.
        """
        try:
            if command['command'] == 'echo':
                return {"response": command['message']}
            elif command['command'] == 'run':
                result = subprocess.run(
                    command['cmd'], shell=True, capture_output=True, text=True)
                return {"response": result.stdout}
            else:
                return {"error": "Unknown command"}
        except Exception as e:
            logging.error(f"Error processing command: {e}")
            return {"error": str(e)}

    async def start_server(self):
        """
        Start the TCP server and accept incoming connections.
        """
        self.server = await asyncio.start_server(self.handle_client, self.host, self.port)
        addr = self.server.sockets[0].getsockname()
        logging.info(f'Serving on {addr}')

        async with self.server:
            await self.server.serve_forever()

    async def stop_server(self):
        """
        Stop the TCP server and disconnect all clients.
        """
        if self.server:
            self.server.close()
            await self.server.wait_closed()
            logging.info('Server stopped')

        for client in self.clients:
            client.close()
            await client.wait_closed()

    def disconnect_client(self, writer: asyncio.StreamWriter):
        """
        Disconnect a client and remove it from the list of active clients.

        Args:
            writer (asyncio.StreamWriter): The stream writer for the client connection.
        """
        if writer in self.clients:
            self.clients.remove(writer)
        self.active_clients -= 1
        writer.close()

    async def broadcast_message(self, message: str):
        """
        Broadcast a message to all connected clients.

        Args:
            message (str): The message to broadcast.
        """
        for client in self.clients:
            client.write((message + '\n').encode())
            await client.drain()

    def log_client_activity(self, client_info, activity):
        """
        Log client connection and disconnection activities.

        Args:
            client_info: Information about the client.
            activity (str): The activity to log (e.g., "connected", "disconnected").
        """
        logging.info(f"Client {client_info} {activity}")


async def main():
    server = JSONTCPServer(host='127.0.0.1', port=8888)
    await server.start_server()

if __name__ == '__main__':
    asyncio.run(main())
